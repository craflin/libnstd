
#pragma once

#include <nstd/Base.h>

class Socket;

class Server
{
public:
  struct Handle;

  struct Event
  {
    enum Type
    {
      interruptType,
      failType,
      openType,
      readType,
      writeType,
      closeType,
      acceptType,
      timerType,
    };
    Type type;
    Handle* handle;
    void* userData;
  };

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

  bool poll(Event& event);
  bool interrupt();

  void suspend(Handle& handle);
  void resume(Handle& handle);

  void setKeepAlive(bool enable = true);
  void setNoDelay(bool enable = true);
  void setSendBufferSize(int size);
  void setReceiveBufferSize(int size);

  void clear();

private:
  Server(const Server&);
  Server& operator=(const Server&);

private:
  class Private;
  Private* p;
};
