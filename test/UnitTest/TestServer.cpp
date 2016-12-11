
#include <nstd/Debug.h>
#include <nstd/Pool.h>
#include <nstd/Socket/Server.h>
#include <nstd/Socket/Socket.h>

void testServer()
{
  // test timer
  {
    Server server;
    int userData;
    Server::Handle* timer = server.createTimer(10, &userData);
    ASSERT(timer);
    class TimerListener : public Event::Listener
    {
    public:
      TimerListener(void* userData, Server* server) : userData(userData), server(server), activations(0) {}
      void handleTimerActivated(Server::Handle& handle, void* userData)
      {
        ASSERT(userData == this->userData);
        if(++activations == 3)
          server->interrupt();
      }
      void* userData;
      Server* server;
      int activations;
    } timerListener(&userData, &server);
    Event::connect(&server, &Server::timerActivated, &timerListener, &TimerListener::handleTimerActivated);
    ASSERT(server.wait());
    ASSERT(timerListener.activations == 3);
    server.close(*timer);
  }

  // test connect fail
  {
    Server server;
    class ClientListener : public Event::Listener
    {
    public:
      ClientListener(Server* server) : activations(0), server(server) {}
      void handleClientFailed(Server::Handle& handle, void* userData)
      {
        ++activations;
        server->interrupt();
      }
      int activations;
      Server* server;
    } clientListener(&server);
    Event::connect(&server, &Server::clientFailed, &clientListener, &ClientListener::handleClientFailed);
    ASSERT(server.connect(Socket::loopbackAddr, 7266, 0));
    ASSERT(server.wait());
    ASSERT(clientListener.activations == 1);
  }

  // test listen, connect, read, write and close
  {
    Server server;
    ASSERT(server.listen(7266, 0));
    enum State
    {
      connecting1,
      connecting2,
      sending1,
      sending2,
      closing1,
      closing2,
    };
    class ServerListener : public Event::Listener
    {
    public:
      ServerListener(Server& server) : server(server), state(connecting1) {}
      void handleClientAccepted(Server::Handle& handle, void* userData)
      {
        ASSERT(state == connecting1 || state == connecting2);
        client2 = server.accept(handle, &client2);
        ASSERT(client2);
        if(state == connecting1)
          state = connecting2;
        else
          state = sending1;
      }
      void handleClientOpened(Server::Handle& handle, void* userData)
      {
        ASSERT(state == connecting1 || state == connecting2);
        ASSERT(&handle == client1);
        ASSERT(userData == &client1);
        ASSERT(server.write(*client1, testData, sizeof(testData)));
        if(state == connecting1)
          state = connecting2;
        else
          state = sending1;
      }
      void handleClientRead(Server::Handle& handle, void* userData)
      {
        ASSERT(state == sending1 || state == sending2 || state == closing1);
        if(state == sending1)
        {
          ASSERT(&handle == client2);
          ASSERT(userData == &client2);
          usize size;
          ASSERT(server.read(*client2, receiveBuffer, sizeof(receiveBuffer), size));
          ASSERT(size == sizeof(testData));
          ASSERT(server.write(*client2, testData, sizeof(testData)));
          state = sending2;
        }
        else if(state == sending2)
        {
          ASSERT(&handle == client1);
          ASSERT(userData == &client1);
          usize size;
          ASSERT(server.read(*client1, receiveBuffer, sizeof(receiveBuffer), size));
          ASSERT(size == sizeof(testData));
          server.close(*client1);
          state = closing1;
        }
        else
        {
          ASSERT(&handle == client2);
          ASSERT(userData == &client2);
          usize size;
          ASSERT(!server.read(*client2, receiveBuffer, sizeof(receiveBuffer), size));
          state = closing2;
        }
      }
      void handleClientClosed(Server::Handle& handle, void* userData)
      {
        ASSERT(state == closing2);
        ASSERT(&handle == client2);
        ASSERT(userData == &client2);
        server.interrupt();
      }
      Server& server;
      State state;
      byte testData[100];
      byte receiveBuffer[231];
      Server::Handle* client1;
      Server::Handle* client2;
    } serverListener(server);
    Event::connect(&server, &Server::clientAccpeted, &serverListener, &ServerListener::handleClientAccepted);
    Event::connect(&server, &Server::clientOpened, &serverListener, &ServerListener::handleClientOpened);
    Event::connect(&server, &Server::clientRead, &serverListener, &ServerListener::handleClientRead);
    Event::connect(&server, &Server::clientClosed, &serverListener, &ServerListener::handleClientClosed);
    serverListener.client1 = server.connect(Socket::loopbackAddr, 7266, &serverListener.client1);
    ASSERT(serverListener.client1);
    ASSERT(server.wait());
  }
}
