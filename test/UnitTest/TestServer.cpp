
#include <nstd/Socket/Server.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Socket/Socket.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Memory.hpp>

void testInterrupt()
{
  Server server;
  server.interrupt();
  server.run();
}

void testTimer()
{
  class Handler : public Server::Timer::ICallback
  {
  public:
    Server _server;
    Server::Timer *_timer;
    usize _activations = 0;
    void onActivated() override
    {
      ++_activations;
      if (_activations == 1)
        return;
      if (_activations == 2)
      {
        _server.remove(*_timer);
        _server.interrupt();
      }
      else
        ASSERT(false);
    }
  } handler;
  handler._timer = handler._server.time(10, handler);
  ASSERT(handler._timer);
  handler._server.run();
}

void testFailingConnect()
{
  class Handler : public Server::Establisher::ICallback
  {
  public:
    Server _server;
    Server::Client::ICallback *onConnected(Server::Client &client) override
    {
      ASSERT(false);
      return 0;
    }
    void onAbolished() { _server.interrupt(); }
  } handler;
  ASSERT(handler._server.connect(Socket::loopbackAddress, 7266, handler));
  handler._server.run();
}

void testListenConnectReadWriteAndClose()
{
  static enum State {
    connecting1,
    connecting2,
    sending1,
    sending2,
    closing1,
    closing2,
  } _state = connecting1;
  static byte _testData[100];
  Memory::fill(_testData, 'a', sizeof(_testData));
  class Client1 : public Server::Client::ICallback
  {
  public:
    Server &_server;
    Server::Client *_client;
    Client1(Server &server) : _server(server) {}
    void onRead() override
    {
      ASSERT(_state == sending2);
      usize size;
      byte receiveBuffer[231];
      ASSERT(_client->read(receiveBuffer, sizeof(receiveBuffer), size));
      ASSERT(size == sizeof(_testData));
      _server.remove(*_client);
      _state = closing1;
    }
    void onWrite() override{};
    void onClosed() override{};
  };
  class Client2 : public Server::Client::ICallback
  {
  public:
    Server &_server;
    Server::Client *_client;
    Client2(Server &server) : _server(server) {}
    void onRead() override
    {
      ASSERT(_state == sending1 || _state == closing1);
      if (_state == sending1)
      {
        usize size;
        byte receiveBuffer[231];
        ASSERT(_client->read(receiveBuffer, sizeof(receiveBuffer), size));
        ASSERT(size == sizeof(_testData));
        ASSERT(_client->write(_testData, sizeof(_testData)));
        _state = sending2;
      }
      else
      {
        usize size;
        byte receiveBuffer[231];
        ASSERT(!_client->read(receiveBuffer, sizeof(receiveBuffer), size));
        _state = closing2;
      }
    }
    void onWrite() override{};
    void onClosed() override
    {
      ASSERT(_state == closing2);
      _server.interrupt();
    };
  };
  class Handler : public Server::Listener::ICallback, public Server::Establisher::ICallback
  {
  public:
    Server _server;
    Client1 _client1;
    Client2 _client2;
    Handler() : _client1(_server), _client2(_server) {}
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override
    {
      ASSERT(_state == connecting1 || _state == connecting2);
      _client2._client = &client;
      if (_state == connecting1)
        _state = connecting2;
      else
        _state = sending1;
      return &_client2;
    }
    Server::Client::ICallback *onConnected(Server::Client &client) override
    {
      ASSERT(_state == connecting1 || _state == connecting2);
      _client1._client = &client;

      ASSERT(_client1._client->write(_testData, sizeof(_testData)));
      if (_state == connecting1)
        _state = connecting2;
      else
        _state = sending1;
      return &_client1;
    }
    void onAbolished() override { ASSERT(false); }
  } handler;
  ASSERT(handler._server.listen(Socket::anyAddress, 7266, handler));
  ASSERT(handler._server.connect(Socket::loopbackAddress, 7266, handler));
  handler._server.run();
}

void testListenConnectConnect()
{
  class Handler : public Server::Listener::ICallback, public Server::Establisher::ICallback, public Server::Client::ICallback
  {
  public:
    Server _server;
    Server::Client *_client1;
    Server::Client *_client2;
    Server::Client *_client3;
    Server::Client *_client4;
    Handler() : _client1(0), _client2(0), _client3(0), _client4(0) {}
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override
    {
      if (!_client2)
        _client2 = &client;
      else
      {
        ASSERT(!_client4);
        _client4 = &client;
      }
      checkShutdown();
      return this;
    }
    Server::Client::ICallback *onConnected(Server::Client &client) override
    {
      if (!_client1)
        _client1 = &client;
      else
      {
        ASSERT(!_client3);
        _client3 = &client;
      }
      checkShutdown();
      return this;
    }
    void onAbolished() override { ASSERT(false); }
    void checkShutdown()
    {
      if (_client1 && _client2 && _client3 && _client4)
        _server.interrupt();
    }
    void onRead() override {}
    void onWrite() override {}
    void onClosed() override {}
  } handler;
  ASSERT(handler._server.listen(Socket::anyAddress, 7266, handler));
  ASSERT(handler._server.connect(Socket::loopbackAddress, 7266, handler));
  ASSERT(handler._server.connect(Socket::loopbackAddress, 7266, handler));
  handler._server.run();
}

void testClientConnect()
{
  static byte _testData[100];
  Memory::fill(_testData, 'a', sizeof(_testData));
  class Handler : public Server::Listener::ICallback, public Server::Client::ICallback
  {
  public:
    Server _server;
    Server::Client *_client;
    bool _received;
    Handler() : _client(0), _received(false) {}
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override
    {
      ASSERT(!_client);
      _client = &client;
      return this;
    }
    void onRead() override
    {
      ASSERT(_client);
      byte receiveData[312];
      usize size;
      if (_received)
      {
        ASSERT(!_client->read(receiveData, sizeof(receiveData), size));
        return;
      }
      ASSERT(_client->read(receiveData, sizeof(receiveData), size));
      _received = true;
      ASSERT(size == sizeof(_testData));
      ASSERT(Memory::compare(&receiveData, &_testData, sizeof(_testData)) == 0);
      ASSERT(_client->write(_testData, sizeof(_testData)));
    }
    void onWrite() override {}
    void onClosed() override
    {
      _server.interrupt();
    }
  } handler;
  ASSERT(handler._server.listen(Socket::anyAddress, 7266, handler));
  Thread thread;
  struct ThreadProc
  {
    static uint threadProc(void *data)
    {
      ThreadProc *threadData = (ThreadProc *)data;
      Socket socket;
      ASSERT(socket.open());
      ASSERT(socket.connect(Socket::loopbackAddress, 7266));
      ASSERT(socket.send(_testData, sizeof(_testData)) == sizeof(_testData));
      byte receiveData[312];
      ASSERT(socket.recv(receiveData, sizeof(receiveData)) == sizeof(_testData));
      ASSERT(Memory::compare(&receiveData, &_testData, sizeof(_testData)) == 0);
      return 32;
    }
  } threadData;
  thread.start(&ThreadProc::threadProc, &threadData);
  handler._server.run();
  ASSERT(thread.join() == 32);
}

void testHostnameResolving()
{
  Socket socket;
  ASSERT(socket.open());
  ASSERT(socket.setReuseAddress());
  ASSERT(socket.bind(Socket::anyAddress, 7266));
  ASSERT(socket.listen());
  class Handler : public Server::Establisher::ICallback 
  {
  public:
    Server _server;
    Server::Client::ICallback *onConnected(Server::Client &client) override
    {
      _server.interrupt();
      return 0;
    }
    void onAbolished() override { ASSERT(false);}
  } handler;
  handler._server.connect(Socket::getHostName(), 7266, handler);
  handler._server.run();
}

int main(int argc, char *argv[])
{
  testInterrupt();
  testTimer();
  testFailingConnect();
  testListenConnectReadWriteAndClose();
  testListenConnectConnect();
  testClientConnect();
  testHostnameResolving();
  return 0;
}
