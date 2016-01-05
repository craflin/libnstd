
#include <nstd/Socket/Socket.h>
#include <nstd/Socket/Server.h>
#include <nstd/Pool.h>
#include <nstd/Error.h>
#include <nstd/Buffer.h>
#include <nstd/List.h>
#include <nstd/MultiMap.h>
#include <nstd/Time.h>

struct Server::Handle
{
  enum Type
  {
    clientType,
    timerType,
    listenerType,
  };

  enum State
  {
    connectingState,
    connectedState,
    suspendedState,
    closingState,
    closedState,
  };

  Type type;
  State state;
  void_t* userData;
};

class Server::Private
{
public:
  class HandleSocket : public Socket
  {
  public:
    Handle* handle;
  };

  struct Listener : public Handle
  {
    HandleSocket socket;
  };

  struct Client : public Handle
  {
    HandleSocket socket;
    Buffer sendBuffer;
  };

  struct Timer : public Handle
  {
    int64_t executionTime;
    int64_t interval;
  };

private:
  Pool<Listener> listeners;
  Pool<Client> clients;
  Pool<Timer> timers;
  MultiMap<int64_t, Timer*> queuedTimers;
  Socket::Poll sockets;
  List<Handle*> closingHandles;

  bool_t keepAlive;
  bool_t noDelay;
  int_t sendBufferSize;
  int_t receiveBufferSize;

public:
  Private() : keepAlive(false), noDelay(false), sendBufferSize(0), receiveBufferSize(0)
  {
    queuedTimers.insert(0, 0); // add default timeout timer
  }

  Handle* listen(uint32_t addr, uint16_t port, void_t* userData)
  {
    Socket socket;
    if(!socket.open() ||
      !socket.setReuseAddress() ||
      !socket.bind(addr, port) ||
      !socket.listen())
      return 0;
    Listener& listener = listeners.append();
    listener.type = Handle::listenerType;
    listener.state = Handle::connectedState;
    listener.socket.swap(socket);
    listener.socket.handle = &listener;
    listener.userData = userData;
    sockets.set(listener.socket, Socket::Poll::acceptFlag);
    return &listener;
  }

  Handle* accept(Handle& handle, void_t* userData, uint32_t* addr, uint16_t* port)
  {
    Listener& listener = (Listener&)handle;
    Socket socket;
    uint32_t addr2;
    uint16_t port2;
    if(!listener.socket.accept(socket, *(addr ? addr : &addr2), *(port ? port : &port2)) ||
      !socket.setNonBlocking() ||
      (keepAlive && !socket.setKeepAlive()) ||
      (noDelay && !socket.setNoDelay()) ||
      (sendBufferSize > 0 && !socket.setSendBufferSize(sendBufferSize)) ||
      (receiveBufferSize > 0 && !socket.setReceiveBufferSize(receiveBufferSize)))
      return 0;
    Client& client = clients.append();
    client.type = Handle::clientType;
    client.state = Handle::connectedState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.userData = userData;
    sockets.set(client.socket, Socket::Poll::readFlag);
    return &client;
  }

  Handle* connect(uint32_t addr, uint16_t port, void_t* userData)
  {
    Socket socket;
    if(!socket.open() ||
      !socket.setNonBlocking() ||
      !socket.connect(addr, port))
      return 0;
    Client& client = clients.append();
    client.type = Handle::clientType;
    client.state = Handle::connectingState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.userData = userData;
    sockets.set(client.socket, Socket::Poll::connectFlag);
    return &client;
  }

  Handle* pair(Socket& otherSocket, void_t* userData)
  {
    Socket socket;
    if(!socket.pair(otherSocket) ||
      !socket.setNonBlocking() ||
      (keepAlive && !socket.setKeepAlive()) ||
      (noDelay && !socket.setNoDelay()) ||
      (sendBufferSize > 0 && !socket.setSendBufferSize(sendBufferSize)) ||
      (receiveBufferSize > 0 && !socket.setReceiveBufferSize(receiveBufferSize)))
    {
      otherSocket.close();
      return 0;
    }
    Client& client = clients.append();
    client.type = Handle::clientType;
    client.state = Handle::connectedState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.userData = userData;
    sockets.set(client.socket, Socket::Poll::readFlag);
    return &client;
  }

  Handle* createTimer(int64_t interval, void_t* userData)
  {
    int64_t executionTime = Time::ticks() + interval;
    Timer& timer = timers.append();
    timer.type = Handle::timerType;
    timer.state = Handle::connectedState;
    timer.userData = userData;
    timer.executionTime = executionTime;
    timer.interval = interval;
    queuedTimers.insert(executionTime, &timer);
    return &timer;
  }

  bool_t write(Handle& handle, const byte_t* data, size_t size, size_t* postponed)
  {
    if(handle.type != Handle::clientType)
      return false;
    Client& client = (Client&)handle;
    if(client.state != Client::connectedState && client.state != Client::suspendedState)
      return false;
    if(client.sendBuffer.isEmpty())
    {
      ssize_t sent = client.socket.send(data, size);
      switch(sent)
      {
      case -1:
        if(Socket::getLastError() == 0) // EWOULDBLOCK
        {
          sent = 0;
          break;
        }
        // no break
      case 0:
        client.state = Client::closingState;
        closingHandles.append(&client);
        return false;
      default:
        break;
      }
      if((size_t)sent >= size)
      {
        if(postponed)
          *postponed = 0;
        return true;
      }
      client.sendBuffer.append(data + sent, size - sent);
      sockets.set(client.socket, Socket::Poll::writeFlag);
    }
    else
      client.sendBuffer.append(data, size);
    if(postponed)
      *postponed = client.sendBuffer.size();
    return true;
  }

  bool_t read(Handle& handle, byte_t* buffer, size_t maxSize, size_t& size)
  {
    if(handle.type != Handle::clientType)
      return false;
    Client& client = (Client&)handle;
    if(client.state != Client::connectedState && client.state != Client::suspendedState)
      return false;
    ssize_t received = client.socket.recv(buffer, maxSize);
    switch(received)
    {
    case -1:
      if(Socket::getLastError() == 0) // EWOULDBLOCK
        return false;
      // no break
    case 0:
      client.state = Client::closingState;
      closingHandles.append(&client);
      return false;
    default:
      break;
    }
    size = (size_t)received;
    return true;
  }

  void_t close(Handle& handle)
  {
    closingHandles.remove(&handle);
    switch(handle.type)
    {
    case Handle::clientType:
      {
        Client& client = (Client&)handle;
        sockets.remove(client.socket);
        clients.remove(client);
      }
      break;
    case Handle::timerType:
      {
        Timer& timer = (Timer&)handle;
        if(timer.state == Handle::connectedState)
          for(MultiMap<int64_t, Timer*>::Iterator i = queuedTimers.find(timer.executionTime), end = queuedTimers.end(); i != end; ++i)
          {
            if(*i == &timer)
            {
              queuedTimers.remove(i);
              break;
            }
            if(i.key() != timer.executionTime)
              break;
          }
        timers.remove(timer);
      }
      break;
    case Handle::listenerType:
      {
        Listener& listener = (Listener&)handle;
        sockets.remove(listener.socket);
        listeners.remove(listener);
      }
      break;
    }
  }

  bool_t poll(Event& event)
  {
    while(!closingHandles.isEmpty())
    {
      Handle& handle = *closingHandles.front();
      closingHandles.removeFront();
      switch(handle.type)
      {
      case Handle::clientType:
        if(handle.state == Handle::closingState)
        {
          Client& client = (Client&)handle;
          event.handle = &client;
          event.userData = client.userData;
          event.type = Event::closeType;
          client.state = Client::closedState;
          sockets.remove(client.socket);
          return true;
        }
        break;
      case Handle::timerType:
        if(handle.state == Handle::suspendedState)
        {
          Timer& timer = (Timer&)handle;
          timer.executionTime += timer.interval;
          queuedTimers.insert(timer.executionTime, &timer);
          timer.state = Handle::connectedState;
        }
        break;
      default:
        break;
      }
    }

    for(Socket::Poll::Event pollEvent;;)
    {
      int64_t now = Time::ticks();
      int64_t timeout = queuedTimers.begin().key() - now;
      for(; timeout <= 0; timeout = queuedTimers.begin().key() - now)
      {
        Timer* timer = queuedTimers.front();
        queuedTimers.removeFront();
        if(timer) // user timer
        {
          event.handle = timer;
          event.userData = timer->userData;
          event.type = Event::timerType;
          timer->state = Handle::suspendedState;
          closingHandles.append(timer);
          return true;
        }
        else
          queuedTimers.insert(now + 300 * 1000, 0); // keep "default timeout" timer
      }

      if(!sockets.poll(pollEvent, timeout))
        break;

      if(!pollEvent.flags)
        continue; // timeout
      event.handle = ((HandleSocket*)pollEvent.socket)->handle;
      event.userData = event.handle->userData;
      if(pollEvent.flags & Socket::Poll::readFlag)
      {
        event.type = Event::readType;
        return true;
      }
      else if(pollEvent.flags & Socket::Poll::writeFlag)
      {
        Client& client = (Client&)*event.handle;
        if(!client.sendBuffer.isEmpty())
        {
          ssize_t sent = client.socket.send(client.sendBuffer, client.sendBuffer.size());
          switch(sent)
          {
          case -1:
            if(Socket::getLastError() == 0) // EWOULDBLOCK
              continue;
            // no break
          case 0:
            client.sendBuffer.free();
            event.type = Event::closeType;
            client.state = Client::closedState;
            sockets.remove(client.socket);
            return true;
          default:
            break;
          }
          client.sendBuffer.removeFront((size_t)sent);
        }
        if(client.sendBuffer.isEmpty())
        {
          client.sendBuffer.free();
          event.type = Event::writeType;
          if(client.state != Client::suspendedState)
            sockets.set(client.socket, Socket::Poll::readFlag);
          else
            sockets.remove(client.socket);
          return true;
        }
        continue;
      }
      else if(pollEvent.flags & Socket::Poll::acceptFlag)
      {
        event.type = Event::acceptType;
        return true;
      }
      else if(pollEvent.flags & Socket::Poll::connectFlag)
      {
        Client& client = *(Client*)event.handle;
        Socket& socket = client.socket;
        int_t error = socket.getAndResetErrorStatus();
        if(error)
        {
          Error::setLastError((uint_t)error);
          event.type = Event::failType;
          client.state = Client::closedState;
          sockets.remove(client.socket);
        }
        else
        {
          if((keepAlive && !socket.setKeepAlive()) ||
            (noDelay && !socket.setNoDelay()) ||
            (sendBufferSize > 0 && !socket.setSendBufferSize(sendBufferSize)) ||
            (receiveBufferSize > 0 && !socket.setReceiveBufferSize(receiveBufferSize)))
          {
            event.type = Event::failType;
            client.state = Client::closedState;
            sockets.remove(client.socket);
          }
          else
          {
            client.state = Client::connectedState;
            event.type = Event::openType;
            sockets.set(*pollEvent.socket, Socket::Poll::readFlag);
          }
        }
        return true;
      }
    }
    return false;
  }

  void_t suspend(Handle& handle)
  {
    if(handle.type != Handle::clientType)
      return;
    Client& client = (Client&)handle;
    if(client.state != Client::connectedState)
      return;
    if(client.sendBuffer.isEmpty())
      sockets.remove(client.socket);
    client.state = Client::suspendedState;
  }

  void_t resume(Handle& handle)
  {
    if(handle.type != Handle::clientType)
      return;
    Client& client = (Client&)handle;
    if(client.state != Client::suspendedState)
      return;
    if(client.sendBuffer.isEmpty())
      sockets.set(client.socket, Socket::Poll::readFlag);
    client.state = Client::connectedState;
  }

  void_t setKeepAlive(bool_t enable) {keepAlive = enable;}
  void_t setNoDelay(bool_t enable) {noDelay = enable;}
  void_t setSendBufferSize(int_t size) {sendBufferSize = size;}
  void_t setReceiveBufferSize(int_t size) {receiveBufferSize = size;}
};

Server::Server() : p(new Private) {}
Server::~Server() {delete p;}
Server::Handle* Server::listen(uint16_t port, void_t* userData) {return p->listen(Socket::anyAddr, port, userData);}
Server::Handle* Server::listen(uint32_t addr, uint16_t port, void_t* userData) {return p->listen(addr, port, userData);}
Server::Handle* Server::connect(uint32_t addr, uint16_t port, void_t* userData) {return p->connect(addr, port, userData);}
Server::Handle* Server::pair(Socket& socket, void_t* userData) {return p->pair(socket, userData);}
Server::Handle* Server::createTimer(int64_t interval, void_t* userData) {return p->createTimer(interval, userData);}
Server::Handle* Server::accept(Handle& handle, void_t* userData, uint32_t* addr, uint16_t* port) {return p->accept(handle, userData, addr, port);}
void_t Server::setUserData(Handle& handle, void_t* userData) {handle.userData = userData;}
void_t* Server::getUserData(Handle& handle) {return handle.userData;}
bool_t Server::write(Handle& handle, const byte_t* data, size_t size, size_t* postponed) {return p->write(handle, data, size, postponed);}
bool_t Server::read(Handle& handle, byte_t* buffer, size_t maxSize, size_t& size) {return p->read(handle, buffer, maxSize, size);}
void_t Server::close(Handle& handle) {return p->close(handle);}
bool_t Server::poll(Event& event) {return p->poll(event);}
void_t Server::suspend(Handle& handle) {return p->suspend(handle);}
void_t Server::resume(Handle& handle) {return p->resume(handle);}
void_t Server::setKeepAlive(bool_t enable) {return p->setKeepAlive(enable);}
void_t Server::setNoDelay(bool_t enable) {return p->setNoDelay(enable);}
void_t Server::setSendBufferSize(int_t size) {return p->setSendBufferSize(size);}
void_t Server::setReceiveBufferSize(int_t size) {return p->setReceiveBufferSize(size);}
