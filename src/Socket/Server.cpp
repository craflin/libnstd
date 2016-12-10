
#include <nstd/Socket/Socket.h>
#include <nstd/Socket/Server.h>
#include <nstd/Pool.h>
#include <nstd/Error.h>
#include <nstd/Buffer.h>
#include <nstd/List.h>
#include <nstd/MultiMap.h>
#include <nstd/Time.h>

class Server::Private
{
public:
  enum Type
  {
    clientType,
    timerType,
    //listenerType,
  };

  class HandleSocket : public Socket
  {
  public:
    void* handle; // ListenerImpl or ClientImpl
  };

  struct ListenerImpl : public Listener
  {
    Private* p;
    HandleSocket socket;
    void* userData;

    void emit(void (Listener::*signal)(void*), void* userData) {Listener::emit(signal, userData);}
  };

  struct ClientImpl : public Client
  {
    Private* p;
    HandleSocket socket;
    Buffer sendBuffer;
    void* userData;
    enum State
    {
      connectingState,
      connectedState,
      suspendedState,
      closingState,
      //closedState,
    } state;

    void emit(void (Client::*signal)(void*), void* userData) {Client::emit(signal, userData);}
  };

  struct TimerImpl : public Timer
  {
    Private* p;
    int64 executionTime;
    int64 interval;
    void* userData;
    bool active;

    void emit(void (Timer::*signal)(void*), void* userData) {Timer::emit(signal, userData);}
  };

private:
  Pool<ListenerImpl> listeners;
  Pool<ClientImpl> clients;
  Pool<TimerImpl> timers;
  MultiMap<int64, TimerImpl*> queuedTimers;
  Socket::Poll sockets;
  Map<void*, Type> closingHandles;

  bool keepAlive;
  bool noDelay;
  int sendBufferSize;
  int receiveBufferSize;

  bool interrupted;

public:
  Private(Server& o) : keepAlive(false), noDelay(false), sendBufferSize(0), receiveBufferSize(0), interrupted(false)
  {
    queuedTimers.insert(0, 0); // add default timeout timer
  }

  Listener* listen(uint32 addr, uint16 port, void* userData)
  {
    Socket socket;
    if(!socket.open() ||
      !socket.setReuseAddress() ||
      !socket.bind(addr, port) ||
      !socket.listen())
      return 0;
    ListenerImpl& listener = listeners.append();
    listener.p = this;
    listener.socket.swap(socket);
    listener.socket.handle = &listener;
    listener.userData = userData;
    sockets.set(listener.socket, Socket::Poll::acceptFlag);
    return &listener;
  }


  Client* connect(uint32 addr, uint16 port, void* userData)
  {
    Socket socket;
    if(!socket.open() ||
      !socket.setNonBlocking() ||
      !socket.connect(addr, port))
      return 0;
    ClientImpl& client = clients.append();
    client.p = this;
    client.state = ClientImpl::connectingState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.userData = userData;
    sockets.set(client.socket, Socket::Poll::connectFlag);
    return &client;
  }

  Client* pair(Socket& otherSocket, void* userData)
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
    ClientImpl& client = clients.append();
    client.p = this;
    client.state = ClientImpl::connectedState;
    client.socket.swap(socket);
    client.socket.handle = &client;
    client.userData = userData;
    sockets.set(client.socket, Socket::Poll::readFlag);
    return &client;
  }

  Timer* createTimer(int64 interval, void* userData)
  {
    int64 executionTime = Time::ticks() + interval;
    TimerImpl& timer = timers.append();
    timer.p = this;
    timer.userData = userData;
    timer.executionTime = executionTime;
    timer.interval = interval;
    queuedTimers.insert(executionTime, &timer);
    timer.active = true;
    return &timer;
  }

  bool wait()
  {
  start:
    while(!closingHandles.isEmpty())
    {
      void* handle = closingHandles.begin().key();
      Type type = closingHandles.front();
      closingHandles.removeFront();
      switch(type)
      {
      case clientType:
        {
          ClientImpl* client = (ClientImpl*)handle;
          ASSERT(client->state == ClientImpl::closingState);
          sockets.remove(client->socket);
          client->emit(&Client::closed, client->userData);
          break;
        }
      case timerType:
        {
          TimerImpl* timer = (TimerImpl*)handle;
          timer->executionTime += timer->interval;
          queuedTimers.insert(timer->executionTime, timer);
          timer->active = true;
          break;
        }
      default:
        ASSERT(false);
        break;
      }
    }

    for(Socket::Poll::Event pollEvent;;)
    {
      int64 now = Time::ticks();
      int64 timeout = queuedTimers.begin().key() - now;
      for(; timeout <= 0; timeout = queuedTimers.begin().key() - now)
      {
        TimerImpl* timer = queuedTimers.front();
        queuedTimers.removeFront();
        if(timer) // user timer
        {
          timer->active = false;
          closingHandles.insert(timer, timerType);
          timer->emit(&Timer::activated, timer->userData);
          goto start;
        }
        else
          queuedTimers.insert(now + 300 * 1000, 0); // keep "default timeout" timer
      }

      if(!sockets.poll(pollEvent, timeout))
        break;

      if(!pollEvent.flags)
      {
        if(interrupted)
        {
          interrupted = false;
          return true;
        }
        continue; // timeout
      }
      if(pollEvent.flags & Socket::Poll::readFlag)
      {
        ClientImpl* client = (ClientImpl*)((HandleSocket*)pollEvent.socket)->handle;
        client->emit(&Client::read, client->userData);
        goto start;
      }
      else if(pollEvent.flags & Socket::Poll::writeFlag)
      {
        ClientImpl* client = (ClientImpl*)((HandleSocket*)pollEvent.socket)->handle;
        if(!client->sendBuffer.isEmpty())
        {
          ssize sent = client->socket.send(client->sendBuffer, client->sendBuffer.size());
          switch(sent)
          {
          case -1:
            if(Socket::getLastError() == 0) // EWOULDBLOCK
              continue;
            // no break
          case 0:
            client->sendBuffer.free();
            client->state = ClientImpl::closingState;
            sockets.remove(client->socket);
            client->emit(&Client::closed, client->userData);
            goto start;
          default:
            break;
          }
          client->sendBuffer.removeFront((usize)sent);
        }
        if(client->sendBuffer.isEmpty())
        {
          client->sendBuffer.free();
          if(client->state != ClientImpl::suspendedState)
            sockets.set(client->socket, Socket::Poll::readFlag);
          else
            sockets.remove(client->socket);
          client->emit(&Client::wrote, client->userData);
          goto start;
        }
        continue;
      }
      else if(pollEvent.flags & Socket::Poll::acceptFlag)
      {
        ListenerImpl* listener = (ListenerImpl*)((HandleSocket*)pollEvent.socket)->handle;
        listener->emit(&Listener::accepted, listener->userData);
        goto start;
      }
      else if(pollEvent.flags & Socket::Poll::connectFlag)
      {
        HandleSocket* socket = (HandleSocket*)pollEvent.socket;
        ClientImpl* client = (ClientImpl*)socket->handle;
        int error = socket->getAndResetErrorStatus();
        if(error)
        {
          Error::setLastError((uint)error);
          client->state = ClientImpl::closingState;
          sockets.remove(*socket);
          client->emit(&Client::failed, client->userData);
          goto start;
        }
        else
        {
          if((keepAlive && !socket->setKeepAlive()) ||
            (noDelay && !socket->setNoDelay()) ||
            (sendBufferSize > 0 && !socket->setSendBufferSize(sendBufferSize)) ||
            (receiveBufferSize > 0 && !socket->setReceiveBufferSize(receiveBufferSize)))
          {
            client->state = ClientImpl::closingState;
            sockets.remove(*socket);
            client->emit(&Client::failed, client->userData);
            goto start;
          }
          else
          {
            client->state = ClientImpl::connectedState;
            sockets.set(*pollEvent.socket, Socket::Poll::readFlag);
            client->emit(&Client::opened, client->userData);
            goto start;
          }
        }
      }
    }
    return false;
  }

  bool interrupt()
  {
    interrupted = true;
    return sockets.interrupt();
  }

  void setKeepAlive(bool enable) {keepAlive = enable;}
  void setNoDelay(bool enable) {noDelay = enable;}
  void setSendBufferSize(int size) {sendBufferSize = size;}
  void setReceiveBufferSize(int size) {receiveBufferSize = size;}

  friend class Listener;
  friend class Client;
  friend class Timer;
};

Server::Client* Server::Listener::accept(void* userData, uint32* addr, uint16* port)
{
  Private::ListenerImpl* listener = (Private::ListenerImpl*)this;
  Private* p = listener->p;
  Socket socket;
  uint32 addr2;
  uint16 port2;
  if(!listener->socket.accept(socket, *(addr ? addr : &addr2), *(port ? port : &port2)) ||
    !socket.setNonBlocking() ||
    (p->keepAlive && !socket.setKeepAlive()) ||
    (p->noDelay && !socket.setNoDelay()) ||
    (p->sendBufferSize > 0 && !socket.setSendBufferSize(p->sendBufferSize)) ||
    (p->receiveBufferSize > 0 && !socket.setReceiveBufferSize(p->receiveBufferSize)))
    return 0;
  Private::ClientImpl& client = p->clients.append();
  client.p = p;
  client.state = Private::ClientImpl::connectedState;
  client.socket.swap(socket);
  client.socket.handle = &client;
  client.userData = userData;
  p->sockets.set(client.socket, Socket::Poll::readFlag);
  return &client;
}

bool Server::Client::write(const byte* data, usize size, usize* postponed)
{
  Private::ClientImpl* client = (Private::ClientImpl*)this;
  if(client->state != Private::ClientImpl::connectedState && client->state != Private::ClientImpl::suspendedState)
    return false;
  if(client->sendBuffer.isEmpty())
  {
    ssize sent = client->socket.send(data, size);
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
      client->state = Private::ClientImpl::closingState;
      client->p->closingHandles.insert(client, Private::clientType);
      return false;
    default:
      break;
    }
    if((usize)sent >= size)
    {
      if(postponed)
        *postponed = 0;
      return true;
    }
    client->sendBuffer.append(data + sent, size - sent);
    client->p->sockets.set(client->socket, Socket::Poll::writeFlag);
  }
  else
    client->sendBuffer.append(data, size);
  if(postponed)
    *postponed = client->sendBuffer.size();
  return true;
}

bool Server::Client::read(byte* buffer, usize maxSize, usize& size)
{
  Private::ClientImpl* client = (Private::ClientImpl*)this;
  if(client->state != Private::ClientImpl::connectedState && client->state != Private::ClientImpl::suspendedState)
    return false;
  ssize received = client->socket.recv(buffer, maxSize);
  switch(received)
  {
  case -1:
    if(Socket::getLastError() == 0) // EWOULDBLOCK
      return false;
    // no break
  case 0:
    client->state = Private::ClientImpl::closingState;
    client->p->closingHandles.insert(client, Private::clientType);
    return false;
  default:
    break;
  }
  size = (usize)received;
  return true;
}

void Server::Client::close()
{
  Private::ClientImpl* client = (Private::ClientImpl*)this;
  Private* p = client->p;
  p->closingHandles.remove(client);
  p->sockets.remove(client->socket);
  p->clients.remove(*client);
}

void Server::Timer::close()
{
  Private::TimerImpl* timer = (Private::TimerImpl*)this;
  Private* p = timer->p;
  p->closingHandles.remove(timer);
  if(timer->active)
    for(MultiMap<int64, Private::TimerImpl*>::Iterator i = p->queuedTimers.find(timer->executionTime), end = p->queuedTimers.end(); i != end; ++i)
    {
      if(*i == timer)
      {
        p->queuedTimers.remove(i);
        break;
      }
      if(i.key() != timer->executionTime)
        break;
    }
  p->timers.remove(*timer);
}

void Server::Listener::close()
{
  Private::ListenerImpl* listener = (Private::ListenerImpl*)this;
  Private* p = listener->p;
  p->sockets.remove(listener->socket);
  p->listeners.remove(*listener);
}

void Server::Client::suspend()
{
  Private::ClientImpl* client = (Private::ClientImpl*)this;
  if(client->state != Private::ClientImpl::connectedState)
    return;
  if(client->sendBuffer.isEmpty())
    client->p->sockets.remove(client->socket);
  client->state = Private::ClientImpl::suspendedState;
}

void Server::Client::resume()
{
  Private::ClientImpl* client = (Private::ClientImpl*)this;
  if(client->state != Private::ClientImpl::suspendedState)
    return;
  if(client->sendBuffer.isEmpty())
    client->p->sockets.set(client->socket, Socket::Poll::readFlag);
  client->state = Private::ClientImpl::connectedState;
}

Server::Server() : p(new Private(*this)) {}
Server::~Server() {delete p;}
Server::Listener* Server::listen(uint16 port, void* userData) {return p->listen(Socket::anyAddr, port, userData);}
Server::Listener* Server::listen(uint32 addr, uint16 port, void* userData) {return p->listen(addr, port, userData);}
Server::Client* Server::connect(uint32 addr, uint16 port, void* userData) {return p->connect(addr, port, userData);}
Server::Client* Server::pair(Socket& socket, void* userData) {return p->pair(socket, userData);}
Server::Timer* Server::createTimer(int64 interval, void* userData) {return p->createTimer(interval, userData);}
bool Server::wait() {return p->wait();}
bool Server::interrupt() {return p->interrupt();}
void Server::setKeepAlive(bool enable) {return p->setKeepAlive(enable);}
void Server::setNoDelay(bool enable) {return p->setNoDelay(enable);}
void Server::setSendBufferSize(int size) {return p->setSendBufferSize(size);}
void Server::setReceiveBufferSize(int size) {return p->setReceiveBufferSize(size);}
