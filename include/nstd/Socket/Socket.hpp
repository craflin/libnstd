
#pragma once

#include <nstd/String.hpp>

class Socket
{
public:
  enum Address
  {
      anyAddress = 0,
      loopbackAddress = 0x7f000001,
      broadcastAddress = 0xffffffff,
  };

  enum Protocol
  {
    tcpProtocol,
    udpProtocol,
  };

public:
  Socket();
  ~Socket();

  bool open(Protocol protocol = tcpProtocol);
  void close();
  bool isOpen() const;
  int64 getFileDescriptor() const;

  void swap(Socket& other);

  bool pair(Socket& other);
  bool accept(Socket& to, uint32& ip, uint16& port);
  bool bind(uint32 ip, uint16 port);
  bool listen();
  bool connect(uint32 ip, uint16 port);

  ssize send(const byte* data, usize size);
  ssize recv(byte* data, usize maxSize, usize minSize = 0);

  ssize sendTo(const byte* data, usize size, uint32 ip, uint16 port);
  ssize recvFrom(byte* data, usize maxSize, uint32& ip, uint16& port);

  bool setKeepAlive();
  bool setReuseAddress();
  bool setNonBlocking();
  bool setNoDelay();
  bool setSendBufferSize(int size);
  bool setReceiveBufferSize(int size);
  bool setBroadcast();

  bool joinMulticastGroup(uint32 ip, uint32 interfaceIp = anyAddress);
  bool setMulticastLoopback(bool enable);

  int getAndResetErrorStatus();

  bool getSockName(uint32& ip, uint16& port);
  bool getPeerName(uint32& ip, uint16& port);

  bool getSockOpt(int level, int optname, void *optval, usize& optlen);

  static void setLastError(int error);
  static int getLastError();
  static String getErrorString(int error = getLastError());
  static uint32 inetAddr(const String& addr, uint16* port = 0);
  static String inetNtoA(uint32 ip);
  static String getHostName();
  static bool getHostByName(const String& host, uint32& addr);

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
      uint flags;
      Socket* socket;
    };

  public:
    Poll();
    ~Poll();

    void set(Socket& socket, uint flags);
    void remove(Socket& socket);

    void clear();

    bool poll(Event& event, int64 timeout);
    bool interrupt();

  private:
    Poll(const Poll&);
    Poll& operator=(const Poll&);

  private:
    class Private;
    Private* p;
  };

private:
#ifdef _WIN32
#ifdef _AMD64
  uint64 s;
#else
  uint s;
#endif
#else
  int s;
#endif

private:
  Socket(const Socket&);
  Socket& operator=(const Socket&);

private:
  class Private;
};
