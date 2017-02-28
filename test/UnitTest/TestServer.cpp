
#include <nstd/Debug.h>
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
    Server::Event event;
    for(int i = 0; i < 3; ++i)
    {
      ASSERT(server.poll(event));
      ASSERT(event.type == Server::Event::timerType);
      ASSERT(event.userData == &userData);
    }
    server.close(*timer);
  }

  // test connect fail
  {
    Server server;
    ASSERT(server.connect(Socket::loopbackAddr, 7266, 0));
    for(Server::Event event; server.poll(event);)
    {
      switch(event.type)
      {
      case Server::Event::failType:
        goto done;
      default:
        break;
      }
    } done: ;
  }

  // test listen, connect, read, write and close
  {
    Server server;
    byte testData[100];
    byte receiveBuffer[231];
    Server::Handle* client1;
    Server::Handle* client2;
    ASSERT(server.listen(7266, 0));
    client1 = server.connect(Socket::loopbackAddr, 7266, &client1);
    ASSERT(client1);
    enum State
    {
      connecting1,
      connecting2,
      sending1,
      sending2,
      closing1,
      closing2,
    } state = connecting1;
    for(Server::Event event; server.poll(event);)
    {
      switch(event.type)
      {
      case Server::Event::acceptType:
        ASSERT(state == connecting1 || state == connecting2);
        client2 = server.accept(*event.handle, &client2);
        ASSERT(client2);
        if(state == connecting1)
          state = connecting2;
        else
          state = sending1;
        break;
      case Server::Event::openType:
        ASSERT(state == connecting1 || state == connecting2);
        ASSERT(event.handle == client1);
        ASSERT(event.userData == &client1);
        ASSERT(server.write(*client1, testData, sizeof(testData)));
        if(state == connecting1)
          state = connecting2;
        else
          state = sending1;
        break;
      case Server::Event::readType:
        ASSERT(state == sending1 || state == sending2 || state == closing1);
        if(state == sending1)
        {
          ASSERT(event.handle == client2);
          ASSERT(event.userData == &client2);
          usize size;
          ASSERT(server.read(*client2, receiveBuffer, sizeof(receiveBuffer), size));
          ASSERT(size == sizeof(testData));
          ASSERT(server.write(*client2, testData, sizeof(testData)));
          state = sending2;
        }
        else if(state == sending2)
        {
          ASSERT(event.handle == client1);
          ASSERT(event.userData == &client1);
          usize size;
          ASSERT(server.read(*client1, receiveBuffer, sizeof(receiveBuffer), size));
          ASSERT(size == sizeof(testData));
          server.close(*client1);
          state = closing1;
        }
        else
        {
          ASSERT(event.handle == client2);
          ASSERT(event.userData == &client2);
          usize size;
          ASSERT(!server.read(*client2, receiveBuffer, sizeof(receiveBuffer), size));
          state = closing2;
        }
        break;
      case Server::Event::closeType:
        ASSERT(state == closing2);
        ASSERT(event.handle == client2);
        ASSERT(event.userData == &client2);
        goto done2;
      default:
        ASSERT(false);
      }
    } done2:;
    ASSERT(state == closing2);
  }

  // test listen, connect, connect
  {
    Server server;
    Server::Handle* client1;
    Server::Handle* client2 = 0;
    Server::Handle* client3;
    Server::Handle* client4 = 0;
    ASSERT(server.listen(7266, 0));
    client1 = server.connect(Socket::loopbackAddr, 7266, &client1);
    ASSERT(client1);
    client3 = server.connect(Socket::loopbackAddr, 7266, &client3);
    ASSERT(client3);
    int openCount = 0;
    for(Server::Event event; server.poll(event);)
    {
      switch(event.type)
      {
      case Server::Event::acceptType:
        if(!client2)
        {
          client2 = server.accept(*event.handle, &client2);
          ASSERT(client2);
        }
        else
        {
          ASSERT(!client4);
          client4 = server.accept(*event.handle, &client2);
          ASSERT(client4);
        }
        if(client4 && openCount == 2)
          goto done3;
        break;
      case Server::Event::openType:
        ASSERT(event.handle == client1 || event.handle == client3);
        ASSERT(event.userData == &client1 || event.userData == &client3);
        ++openCount;
        if(client4 && openCount == 2)
          goto done3;
        break;
      default:
        ASSERT(false);
      }
    } done3:;
    ASSERT(client4 && openCount == 2);
  }

  // test interrupt
  {
    Server server;
    ASSERT(server.interrupt());
    Server::Event event;
    ASSERT(server.poll(event));
    ASSERT(event.type == Server::Event::interruptType);
  }
}
