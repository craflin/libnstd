
#pragma once

#include <nstd/Event.h>

class Socket;

class Server : public Event::Source
{
public:
  struct Handle;

public: // events
  void clientFailed(Handle& handle, void* userData) {}
  void clientOpened(Handle& handle, void* userData) {}
  void clientRead(Handle& handle, void* userData) {}
  void clientWrote(Handle& handle, void* userData) {}
  void clientClosed(Handle& handle, void* userData) {}
  void clientAccpeted(Handle& handle, void* userData) {}
  void timerActivated(Handle& handle, void* userData) {}

public:
  Server();
  ~Server();

  Handle* listen(uint16 port, void* userData);
  Handle* listen(uint32 addr, uint16 port, void* userData);
  Handle* connect(uint32 addr, uint16 port, void* userData);
  Handle* pair(Socket& socket, void* userData);
  Handle* createTimer(int64 interval, void* userData);
  Handle* accept(Handle& handle, void* userData, uint32* addr = 0, uint16* port = 0);

  void setUserData(Handle& handle, void* userData);
  void* getUserData(Handle& handle);

  bool write(Handle& handle, const byte* data, usize size, usize* postponed = 0);
  bool read(Handle& handle, byte* buffer, usize maxSize, usize& size);

  void close(Handle& handle);

  bool wait();
  bool interrupt();

  void suspend(Handle& handle);
  void resume(Handle& handle);

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
