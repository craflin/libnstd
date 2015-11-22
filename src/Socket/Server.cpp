
#include <nstd/Socket/Socket.h>
#include <nstd/Socket/Server.h>
#include <nstd/Pool.h>
#include <nstd/Error.h>
#include <nstd/Buffer.h>
#include <nstd/HashSet.h>
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
  Type type;
};

class Server::Private
{
public:
  class HandleSocket : public Socket
  {
  public:
    Handle* handle;
    void_t* userData;
  };

  struct Listener : public Handle
  {
    HandleSocket socket;
  };

  struct Client : public Handle
  {
    enum State
    {
      connectingState,
      connectedState,
      suspendedState,
      closingState,
      closedState,
    };

    HandleSocket socket;
    Buffer sendBuffer;
    State state;
  };

  struct Timer : public Handle
  {
    void_t* userData;
    int64_t executionTime;
    int64_t interval;
  };

public:
  Pool<Listener> listeners;
  Pool<Client> clients;
  Pool<Timer> timers;
  MultiMap<int64_t, Timer*> sortedTimers;
  Socket::Poll sockets;
  HashSet<Handle*> closingHandles;

public:
  Handle* listen(uint16_t port, void_t* userData)
  {
    Socket socket;
    if(!socket.open() ||
      !socket.setReuseAddress() ||
      !socket.bind(Socket::anyAddr, port) ||
      !socket.listen())
      return 0;
    Listener& listener = listeners.append();
    listener.type = Handle::listenerType;
    listener.socket.swap(socket);
    listener.socket.handle = &listener;
    listener.socket.userData = userData;
    sockets.set(listener.socket, Socket::Poll::acceptFlag);
    return &listener;
  }

  Handle* accept(Handle& handle, void_t* userData)
  {
    Listener& listener = (Listener&)handle;
    Socket socket;
    uint32_t ip;
    uint16_t port;
    if(!listener.socket.accept(socket, ip, port) ||
      !listener.socket.setNonBlocking() ||
      !listener.socket.setKeepAlive() ||
      !listener.socket.setNoDelay())
      return 0;
    Client& client = clients.append();
    client.type = Handle::clientType;
    client.state = Client::connectedState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.socket.userData = userData;
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
    client.state = Client::connectingState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.socket.userData = userData;
    sockets.set(client.socket, Socket::Poll::connectFlag);
    return &client;
  }

  Handle* pair(Socket& otherSocket, void_t* userData)
  {
    Socket socket;
    if(!socket.pair(otherSocket) ||
      !socket.setNonBlocking() ||
      !socket.setKeepAlive() ||
      !socket.setNoDelay())
    return 0;
    Client& client = clients.append();
    client.type = Handle::clientType;
    client.state = Client::connectedState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.socket.userData = userData;
    sockets.set(client.socket, Socket::Poll::readFlag);
    return &client;
  }

  Handle* createTimer(int64_t interval, void_t* userData)
  {
    int64_t executionTime = Time::ticks() + interval;
    Timer& timer = timers.append();
    timer.type = Handle::timerType;
    timer.userData = userData;
    timer.executionTime = executionTime;
    timer.interval = interval;
    sortedTimers.insert(executionTime, &timer);
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
    }
    size = received;
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
        for(MultiMap<int64_t, Timer*>::Iterator i = sortedTimers.find(timer.executionTime), end = sortedTimers.end(); i != end; ++i)
        {
          if(*i == &timer)
          {
            sortedTimers.remove(i);
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
        {
          Client& client = (Client&)handle;
          if(client.state == Client::closingState)
          {
            event.handle = &client;
            event.userData = client.socket.userData;
            event.type = Event::closeType;
            client.state = Client::closedState;
            sockets.remove(client.socket);
            return true;
          }
        }
        break;
      }
    }

    for(Socket::Poll::Event pollEvent;;)
    {
      int64_t timeout = sortedTimers.isEmpty() ? 300 * 1000 : sortedTimers.begin().key() - Time::ticks();

      if(timeout <= 0)
      {
        Timer& timer = *sortedTimers.front();
        sortedTimers.removeFront();
        timer.executionTime += timer.interval;
        sortedTimers.insert(timer.executionTime, &timer);
        event.handle = &timer;
        event.userData = timer.userData;
        event.type = Event::timerType;
        return true;
      }

      if(!sockets.poll(pollEvent, timeout))
        break;

      if(!pollEvent.flags)
        continue; // timeout
      event.handle = ((HandleSocket*)pollEvent.socket)->handle;
      event.userData = ((HandleSocket*)pollEvent.socket)->userData;
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
          }
          client.sendBuffer.removeFront(sent);
        }
        if(client.sendBuffer.isEmpty())
        {
          client.sendBuffer.free();
          event.type = Event::writeType;
          if(client.state != Client::suspendedState)
            sockets.set(*pollEvent.socket, Socket::Poll::readFlag);
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
        int error = pollEvent.socket->getAndResetErrorStatus();
        if(error)
        {
          Error::setLastError(error);
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
};

Server::Server() : p(new Private) {}
Server::~Server() {delete p;}
Server::Handle* Server::listen(uint16_t port, void_t* userData) {return p->listen(port, userData);}
Server::Handle* Server::connect(uint32_t addr, uint16_t port, void_t* userData) {return p->connect(addr, port, userData);}
Server::Handle* Server::pair(Socket& socket, void_t* userData) {return p->pair(socket, userData);}
Server::Handle* Server::accept(Handle& handle, void_t* userData) {return p->accept(handle, userData);}
Server::Handle* Server::createTimer(int64_t interval, void_t* userData) {return p->createTimer(interval, userData);}
bool_t Server::write(Handle& handle, const byte_t* data, size_t size, size_t* postponed) {return p->write(handle, data, size, postponed);}
bool_t Server::read(Handle& handle, byte_t* buffer, size_t maxSize, size_t& size) {return p->read(handle, buffer, maxSize, size);}
void_t Server::close(Handle& handle) {return p->close(handle);}
bool_t Server::poll(Event& event) {return p->poll(event);}
void_t Server::suspend(Handle& handle) {return p->suspend(handle);}
void_t Server::resume(Handle& handle) {return p->resume(handle);}
