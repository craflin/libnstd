
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
    void_t* userData;
  };

public:
  Server();
  ~Server();

  Handle* listen(uint16_t port, void_t* userData);
  Handle* connect(uint32_t addr, uint16_t port, void_t* userData);
  Handle* pair(Socket& socket, void_t* userData);
  Handle* createTimer(int64_t interval, void_t* userData);

  Handle* accept(Handle& handle, void_t* userData, uint32_t* addr = 0, uint16_t* port = 0);

  bool_t write(Handle& handle, const byte_t* data, size_t size, size_t* postponed = 0);
  bool_t read(Handle& handle, byte_t* buffer, size_t maxSize, size_t& size);

  void_t close(Handle& handle);

  bool_t poll(Event& event);

  void_t suspend(Handle& handle);
  void_t resume(Handle& handle);

  void_t setKeepAlive(bool_t enable = true);
  void_t setNoDelay(bool_t enable = true);
  void_t setSendBufferSize(int_t size);
  void_t setReceiveBufferSize(int_t size);

private:
  Server(const Server&);
  Server& operator=(const Server&);

private:
  class Private;
  Private* p;
};
