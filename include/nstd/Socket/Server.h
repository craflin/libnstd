
#pragma once

#include <nstd/Event.h>

class Socket;

class Server
{
public:
  class Client : public Event::Source
  {
  public: // events
    void failed(void* userData) {}
    void opened(void* userData) {}
    void read(void* userData) {}
    void wrote(void* userData) {}
    void closed(void* userData) {}

  public:
    bool write(const byte* data, usize size, usize* postponed = 0);
    bool read(byte* buffer, usize maxSize, usize& size);
    void suspend();
    void resume();
    void close();
  };

  class Listener : public Event::Source
  {
  public: // events
    void accepted(void* userData) {}

  public:
    Client* accept(void* userData, uint32* addr = 0, uint16* port = 0);
    void close();
  };

  class Timer : public Event::Source
  {
  public: // events
    void activated(void* userData) {}

  public:
    void close();
  };

public:
  Server();
  ~Server();

  Listener* listen(uint16 port, void* userData);
  Listener* listen(uint32 addr, uint16 port, void* userData);
  Client* connect(uint32 addr, uint16 port, void* userData);
  Client* pair(Socket& socket, void* userData);
  Timer* createTimer(int64 interval, void* userData);
  Client* accept(Listener& listener, void* userData, uint32* addr = 0, uint16* port = 0);

  bool wait();
  bool interrupt();

  void setKeepAlive(bool enable = true);
  void setNoDelay(bool enable = true);
  void setSendBufferSize(int size);
  void setReceiveBufferSize(int size);

private:
  Server(const Server&);
  Server& operator=(const Server&);

private:
  class Private;
  Private* p;
};
