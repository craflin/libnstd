
#pragma once

#include <nstd/String.h>

class Socket
{
public:
  enum Addr
  {
      anyAddr = 0,
      loopbackAddr = 0x7f000001,
  };

public:
  Socket();
  ~Socket();

  bool_t open();
  void_t close();
  bool_t isOpen() const;

  void_t swap(Socket& other);

  bool_t pair(Socket& other);
  bool_t accept(Socket& to, uint32_t& ip, uint16_t& port);
  bool_t bind(uint32_t ip, uint16_t port);
  bool_t listen();
  bool_t connect(uint32_t ip, uint16_t port);

  ssize_t send(const byte_t* data, size_t size);
  ssize_t recv(byte_t* data, size_t maxSize, size_t minSize = 0);

  bool_t setKeepAlive();
  bool_t setReuseAddress();
  bool_t setNonBlocking();
  bool_t setNoDelay();
  bool_t setSendBufferSize(int_t size);
  bool_t setReceiveBufferSize(int_t size);

  int_t getAndResetErrorStatus();

  bool_t getSockName(uint32_t& ip, uint16_t& port);
  bool_t getPeerName(uint32_t& ip, uint16_t& port);

  static void_t setLastError(int_t error);
  static int_t getLastError();
  static String getErrorString(int_t error = getLastError());
  static uint32_t inetAddr(const String& addr);
  static String inetNtoA(uint32_t ip);

public:
  class Poll
  {
  public:
    enum Flag
    {
      readFlag = 0x01,
      writeFlag = 0x02,
      acceptFlag = 0x04,
      connectFlag = 0x08,
    };

    struct Event
    {
      uint_t flags;
      Socket* socket;
    };

  public:
    Poll();
    ~Poll();

    void_t set(Socket& socket, uint_t flags);
    void_t remove(Socket& socket);

    bool_t poll(Event& event, int64_t timeout);

  private:
    class Private;
    Private* p;
  };

private:
#ifdef _WIN32
#ifdef _AMD64
  uint64_t s;
#else
  uint_t s;
#endif
#else
  int_t s;
#endif

private:
  Socket(const Socket&);
  Socket& operator=(const Socket&);

private:
  class Private;
};
