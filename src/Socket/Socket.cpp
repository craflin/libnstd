
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS // todo: get rid of this
#define _CRT_NO_POSIX_ERROR_CODES
#include <winsock2.h>
#include <Ws2tcpip.h>
#ifdef UNICODE
#include <Mstcpip.h>
#undef ASSERT
#endif
#ifdef EINVAL
#undef EINVAL
#endif
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#if defined(__linux__)
#include <sys/epoll.h>
#else
#include <poll.h>
#endif
#include <sys/eventfd.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#endif

#ifdef _WIN32
#define ERRNO WSAGetLastError()
#define SET_ERRNO(e) WSASetLastError(e)
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEINPROGRESS
#define EINVAL WSAEINVAL
#define CLOSE closesocket
typedef int socklen_t;
#define MSG_NOSIGNAL 0
#else
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define ERRNO errno
#define SET_ERRNO(e) (errno = e)
#define CLOSE close
#define SOCKET_ERROR (-1)
#endif

#include <nstd/Debug.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Array.hpp>
#include <nstd/Socket/Socket.hpp>
#ifdef _WIN32
#include <nstd/List.hpp>
#include <nstd/Map.hpp>
#endif

class Socket::Private
{
#ifdef _WIN32
  static class Winsock
  {
  public:
    Winsock()
    {
      WORD wVersionRequested = MAKEWORD(2, 2);
      WSADATA wsaData;
      WSAStartup(wVersionRequested, &wsaData);
    }
    ~Winsock()
    {
      WSACleanup();
    }
  } winsock;
#endif
public:
  static const int typeMap[2];
  static const int protocolMap[2];
};

#ifdef _WIN32
Socket::Private::Winsock Socket::Private::winsock;
#endif

const int Socket::Private::typeMap[2] = {SOCK_STREAM, SOCK_DGRAM};
const int Socket::Private::protocolMap[2] = {IPPROTO_TCP, IPPROTO_UDP};

Socket::Socket() : s(INVALID_SOCKET) {}

Socket::~Socket()
{
  if(s != INVALID_SOCKET)
    ::CLOSE(s);
}

bool Socket::open(Protocol protocol)
{
  if(s != INVALID_SOCKET)
    close();

#ifdef _WIN32
  s = WSASocket(AF_INET, Private::typeMap[protocol], Private::protocolMap[protocol], NULL, 0, WSA_FLAG_OVERLAPPED);
#else
  s = socket(AF_INET, Private::typeMap[protocol] | SOCK_CLOEXEC, Private::protocolMap[protocol]);
#endif
  if(s == INVALID_SOCKET)
    return false;
  return true;
}

void Socket::close()
{
  if(s != INVALID_SOCKET)
  {
    ::CLOSE(s);
    s = INVALID_SOCKET;
  }
}

bool Socket::isOpen() const
{
  return s != INVALID_SOCKET;
}

int64 Socket::getFileDescriptor() const
{
    return (int64)s;
}

void Socket::swap(Socket& other)
{
  SOCKET tmp = s;
  s = other.s;
  other.s = tmp;
}

bool Socket::pair(Socket& other)
{
  if(s != INVALID_SOCKET)
    close();
  if(other.s != INVALID_SOCKET)
    other.close();

#ifdef _WIN32
  SOCKET serv, conn, cli;
  int err;
  for(;;)
  {
    // create listener socket
    serv = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if(serv == INVALID_SOCKET)
      return false;

    // set reuse addr flag
    int val = 1;
    if(setsockopt(serv, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) == SOCKET_ERROR)
      goto closeServ;

    // bind to loopback
    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(::bind(serv, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
      goto closeServ;

    // get port number
    socklen_t len = sizeof(sin);
    if(getsockname(serv, (sockaddr*)&sin, &len) == SOCKET_ERROR)
      goto closeServ;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_family = AF_INET;
    if(::listen(serv, 1) == SOCKET_ERROR)
      goto closeServ;

    // create connect socket
    conn = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if(conn == INVALID_SOCKET)
      goto closeServ;

    // connect
    if(::connect(conn, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
      goto closeConn;

    // accept
    cli = ::accept(serv, (sockaddr *)&sin, &len);
    if(cli == INVALID_SOCKET)
      goto closeConn;

    // check sockets are connected
    sockaddr_in sin2;
    if(getsockname(conn, (sockaddr*)&sin2, &len) == SOCKET_ERROR)
      goto closeCli;
    if(sin2.sin_port != sin.sin_port)
    {
      closesocket(cli);
      closesocket(conn);
      closesocket(serv);
      continue;
    }

    closesocket(serv);
    s = cli;
    other.s = conn;
    return true;
  }

closeCli:
  err = WSAGetLastError();
  closesocket(cli);
  WSASetLastError(err);
closeConn:
  err = WSAGetLastError();
  closesocket(conn);
  WSASetLastError(err);
closeServ:
  err = WSAGetLastError();
  closesocket(serv);
  WSASetLastError(err);
  return false;
#else
  int fd[2];
  if(socketpair(PF_LOCAL, SOCK_STREAM, 0, fd) != 0)
    return false;
  s = fd[0];
  other.s = fd[1];
  return true;
#endif
}

bool Socket::accept(Socket& to, uint32& ip, uint16& port)
{
  if(to.s != INVALID_SOCKET)
    to.close();

  struct sockaddr_in sin;
  socklen_t val = sizeof(sin);

#ifdef _WIN32
  to.s = ::accept(s, (struct sockaddr *)&sin, &val);
#else
  to.s = ::accept4(s, (struct sockaddr *)&sin, &val, SOCK_CLOEXEC);
#endif
  if(to.s == INVALID_SOCKET)
    return false;
  port = ntohs(sin.sin_port);
  ip = ntohl(sin.sin_addr.s_addr);
  return true;
}

bool Socket::setNonBlocking()
{
#ifdef _WIN32
  u_long val = 1;
  if(ioctlsocket(s, FIONBIO, &val) != 0)
#else
  if(fcntl(s, F_SETFL, O_NONBLOCK) != 0)
#endif
    return false;
  return true;
}

bool Socket::setNoDelay()
{
  int val = 1;
  if(setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val)) != 0)
    return false;
  return true;
}

bool Socket::setSendBufferSize(int size)
{
  if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) != 0)
    return false;
  return true;
}

bool Socket::setReceiveBufferSize(int size)
{
  if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) != 0)
    return false;
  return true;
}

bool Socket::setBroadcast()
{
  int val = 1;
  if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof(val)) != 0)
    return false;
  return true;
}

bool Socket::joinMulticastGroup(uint32 ip, uint32 interfaceIp)
{
  ip_mreq mreq;
  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr.s_addr = htonl(ip);
  mreq.imr_interface.s_addr = htonl(interfaceIp);
  if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) != 0)
    return false;
  return true;
}

bool Socket::setMulticastLoopback(bool enable)
{
  int val = enable ? 1 : 0;
  if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&val, sizeof(val)) != 0)
    return false;
  return true;
}

bool Socket::setKeepAlive()
{
  int val = 1;
  if(setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(val)) != 0)
    return false;
  return true;
}

bool Socket::setReuseAddress()
{
  int val = 1;
  if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) != 0)
    return false;
  return true;
}
/*
bool Socket::setReusePort()
{
  int val = 1;
#ifdef _WIN32
  if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val)) != 0)
#else
  if(setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char*)&val, sizeof(val)) != 0)
#endif
    return false;
  return true;
}
*/
bool Socket::bind(uint32 ip, uint16 port)
{
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl((ip == INADDR_NONE) ? INADDR_ANY : ip);

  if(sin.sin_addr.s_addr == htonl(INADDR_ANY) && port == 0)
    return true;

  if(::bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    return false;
  return true;
}

bool Socket::listen()
{
  if(::listen(s, SOMAXCONN) < 0)
    return false;
  return true;
}

bool Socket::connect(uint32 ip, uint16 port)
{
  struct sockaddr_in sin;

  memset(&sin,0,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(ip);

  if(::connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
  {
    if(ERRNO && ERRNO != EINPROGRESS
#ifdef _WIN32
      && ERRNO != EWOULDBLOCK
#endif
      )
      return false;
  }

  return true;
}

int Socket::getAndResetErrorStatus()
{
  if(s == INVALID_SOCKET)
  {
    SET_ERRNO(EINVAL);
    return EINVAL;
  }
  int optVal;
  socklen_t optLen = sizeof(int);
  if(getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) != 0)
    return ERRNO;
  return optVal;
}

bool Socket::getSockName(uint32& ip, uint16& port)
{
  sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if(getsockname(s, (sockaddr*)&addr, &len) != 0)
    return false;
  ip = ntohl(addr.sin_addr.s_addr);
  port = ntohs(addr.sin_port);
  return true;
}

bool Socket::getPeerName(uint32& ip, uint16& port)
{
  sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if(getpeername(s, (sockaddr*)&addr, &len) != 0)
    return false;
  ip = ntohl(addr.sin_addr.s_addr);
  port = ntohs(addr.sin_port);
  return true;
}

bool Socket::getSockOpt(int level, int optname, void *optval, usize& optlen)
{
  socklen_t len = (socklen_t)optlen;
  if(getsockopt(s, level, optname, (char*)optval, &len) != 0)
    return false;
  optlen = len;
  return true;
}

ssize Socket::send(const byte* data, usize size)
{
  ssize r = ::send(s, (const char*)data, (int)size, MSG_NOSIGNAL);
  if(r == SOCKET_ERROR)
  {
    if(ERRNO == EWOULDBLOCK 
#ifndef _WIN32
      || ERRNO == EAGAIN
#endif
      )
    {
      SET_ERRNO(0);
    }
    return -1;
  }
  return r;
}

ssize Socket::sendTo(const byte* data, usize size, uint32 ip, uint16 port)
{
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htonl(ip);

  ssize r = ::sendto(s, (const char*)data, (int)size, 0, (sockaddr*)&sin, sizeof(sin));
  if(r == SOCKET_ERROR)
  {
    if(ERRNO == EWOULDBLOCK 
#ifndef _WIN32
      || ERRNO == EAGAIN
#endif
      )
    {
      SET_ERRNO(0);
    }
    return -1;
  }
  return r;
}

ssize Socket::recvFrom(byte* data, usize maxSize, uint32& ip, uint16& port)
{
  struct sockaddr_in sin;
  socklen_t sinSize = sizeof(sin);

  ssize r = ::recvfrom(s, (char*)data, (int)maxSize, 0, (sockaddr*)&sin, &sinSize);
  if(r == SOCKET_ERROR)
  {
    if(ERRNO == EWOULDBLOCK 
#ifndef _WIN32
      || ERRNO == EAGAIN
#endif
      )
    {
      SET_ERRNO(0);
    }
    return -1;
  }
  ip = ntohl(sin.sin_addr.s_addr);
  port = ntohs(sin.sin_port);
  return r;
}

ssize Socket::recv(byte* data, usize maxSize, usize minSize)
{
  ssize r = ::recv(s, (char*)data, (int)maxSize, 0);
  switch(r)
  {
  case SOCKET_ERROR:
    if(ERRNO == EWOULDBLOCK 
#ifndef _WIN32
      || ERRNO == EAGAIN
#endif
      )
    {
      SET_ERRNO(0);
    }
    return -1;
  case 0:
    return 0;
  default:
    break;
  }
  if((usize)r >= minSize)
    return r;
  usize received = (usize)r;
  for(;;)
  {
    r = ::recv(s, (char*)data + received, (int)(maxSize - received), 0);
    switch(r)
    {
    case SOCKET_ERROR:
      if(ERRNO == EWOULDBLOCK 
  #ifndef _WIN32
        || ERRNO == EAGAIN
  #endif
        )
      {
        SET_ERRNO(0);
        return received;
      }
      return -1;
    case 0:
      return 0;
    default:
      break;
    }
    received += r;
    if(received >= minSize)
      return received;
  }
}

void Socket::setLastError(int error)
{
  SET_ERRNO(error);
}

int Socket::getLastError()
{
  return ERRNO;
}

String Socket::getErrorString(int error)
{
#ifdef _WIN32
  TCHAR errorMessage[256];
  DWORD len = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) errorMessage,
        256, NULL );
  ASSERT(len >= 0 && len <= 256);
  while(len > 0 && isspace(((unsigned char*)errorMessage)[len - 1]))
    --len;
  errorMessage[len] = '\0';
  return String(errorMessage, len);
#else
  const char* errorMessage = ::strerror(error);
  return String(errorMessage, String::length(errorMessage));
#endif
}

uint32 Socket::inetAddr(const String& addr, uint16* port)
{
  const tchar* portStr = addr.find(':');
  if(portStr)
  {
    if(port)
      *port = (uint16)String::toUInt(portStr + 1);
#ifdef UNICODE
    in_addr inaddr;
    LPCWSTR end; 
    if(RtlIpv4StringToAddress(addr, FALSE, &end, &inaddr) != 0)
      return ntohl(INADDR_NONE);
    return ntohl(inaddr.s_addr);
#else
    return ntohl(inet_addr((const tchar*)addr.substr(0, (portStr - (const tchar*)addr) / sizeof(tchar))));
#endif
  }
#ifdef UNICODE
  in_addr inaddr;
  LPCWSTR end; 
  if(RtlIpv4StringToAddress(addr, FALSE, &end, &inaddr) != 0)
    return ntohl(INADDR_NONE);
  return ntohl(inaddr.s_addr);
#else
  return ntohl(inet_addr((const tchar*)addr));
#endif
}

String Socket::inetNtoA(uint32 ip)
{
#if defined(_WIN32) && defined(UNICODE)
  in_addr in;
  in.s_addr = htonl(ip);
  tchar buf[17];
  buf[0] = _T('\0');
  RtlIpv4AddressToString(&in, buf);
  return String(buf, String::length(buf));
#else
  return String::fromPrintf("%hu.%hu.%hu.%hu", (uint16)(ip >> 24),  (uint16)((ip >> 16) & 0xff), (uint16)((ip >> 8) & 0xff), (uint16)(ip & 0xff));
#endif
}

String Socket::getHostName()
{
  char name[256];
  if(gethostname(name, sizeof(name)) != 0)
    return String();
#ifdef UNICODE
  WCHAR wname[256];
  int size = MultiByteToWideChar(CP_ACP, 0, name, -1, wname, sizeof(wname)/sizeof(WCHAR));
  if(!size)
    return String();
  return String(wname, size - 1);
  //return String::fromPrintf(_T("%hu"), name);
#else
  return String(name, String::length(name));
#endif
}

bool Socket::getHostByName(const String& host, uint32& addr)
{
#ifdef _WIN32
  ADDRINFOT hints = {0};
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_IPV4;
  ADDRINFOT* ai;
  if (GetAddrInfo((const tchar*)host, NULL, &hints, &ai) != 0)
    return false;
  for (ADDRINFOT* res = ai; res; res = res->ai_next)
    if(res->ai_family == AF_INET)
    {
      addr = ntohl(((sockaddr_in*)res->ai_addr)->sin_addr.s_addr);
      FreeAddrInfo(ai);
      return true;
    }
  FreeAddrInfo(ai);
#else
  addrinfo hints = {0};
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_IP;
  addrinfo* ai;
  if (getaddrinfo((const tchar*)host, NULL, &hints, &ai) != 0)
    return false;
  for (addrinfo* res = ai; res; res = res->ai_next)
    if(res->ai_family == AF_INET)
    {
      addr = ntohl(((sockaddr_in*)res->ai_addr)->sin_addr.s_addr);
      freeaddrinfo(ai);
      return true;
    }
  freeaddrinfo(ai);
#endif
    return false;
}

#ifdef _WIN32

class Socket::Poll::Private
{
public:
  Private();
  ~Private();

  inline void clear();
  inline void set(Socket& socket, uint events);
  inline void remove(Socket& socket);
  inline bool poll(Event& event, int64 timeout);
  inline bool interrupt();

public:
  struct Overlapped : public WSAOVERLAPPED
  {
    uint event;
  };

  struct SocketInfo
  {
    Socket* socket;
    ULONG_PTR key;
    uint events;
    SOCKET s;
    bool completionPortCreated;
  };

  struct WaitThreadMessage
  {
    enum Type
    {
      quit,
      addConnect,
      addAccept,
      remove
    } type;
    SOCKET s;
    ULONG_PTR key;
  };

  HANDLE completionPort;
  HashMap<Socket*, SocketInfo> sockets;
  HashMap<ULONG_PTR, SocketInfo*> keys;
  ULONG_PTR nextKey;

  SocketInfo* detachedSockInfo;
  uint detachedSocketEvent;

  HANDLE waitThread;
  HANDLE waitThreadEvent;
  CRITICAL_SECTION waitThreadMutex;
  List<WaitThreadMessage> waitThreadQueue;

  Overlapped connectOverlapped;
  Overlapped acceptOverlapped;
  Overlapped readOverlapped;
  Overlapped writeOverlapped;
  Overlapped interruptOverlapped;

private:
  void pushWaitThreadMessage(const WaitThreadMessage& message);
  void joinWaitThread();

  static uint waitThreadProc(Private* p);
};

Socket::Poll::Private::Private() : nextKey(1), detachedSockInfo(0), waitThread(0), waitThreadEvent(0)
{
  completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
  InitializeCriticalSection(&waitThreadMutex);
  ZeroMemory(&readOverlapped, sizeof(readOverlapped));
  ZeroMemory(&writeOverlapped, sizeof(writeOverlapped));
  connectOverlapped.event = connectFlag;
  acceptOverlapped.event = acceptFlag;
  readOverlapped.event = readFlag;
  writeOverlapped.event = writeFlag;
  interruptOverlapped.event = 0;
}

Socket::Poll::Private::~Private()
{
  joinWaitThread();
  DeleteCriticalSection(&waitThreadMutex);

  if(completionPort)
    CloseHandle(completionPort);
}

void Socket::Poll::Private::clear()
{
  if(nextKey != 1)
  {
    joinWaitThread();
    waitThreadQueue.clear();
    sockets.clear();
    keys.clear();
    detachedSockInfo = 0;
    if(completionPort)
      CloseHandle(completionPort);
    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
    nextKey = 1;
  }
}

void Socket::Poll::Private::set(Socket& socket, uint events)
{
  Private::SocketInfo* sockInfo;
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it == sockets.end()) // this is a new one, lets create a key for it and attach it to the completion port
  {
    if(socket.s == INVALID_SOCKET)
      return;
    sockInfo = &sockets.append(&socket, Private::SocketInfo());
    sockInfo->key = nextKey++;
    sockInfo->socket = &socket;
    sockInfo->events = 0;
    sockInfo->s = socket.s;
    sockInfo->completionPortCreated = false;
    keys.append(sockInfo->key, sockInfo);
  }
  else
  {
    sockInfo = &*it;
    uint removedEvents = sockInfo->events & ~events;
    if(removedEvents)
    {
      if(removedEvents & (connectFlag | acceptFlag))
      {
        Private::WaitThreadMessage removeMessage = {Private::WaitThreadMessage::remove, sockInfo->s, sockInfo->key};
        pushWaitThreadMessage(removeMessage);
      }
      if(detachedSockInfo == sockInfo)
      {
        detachedSocketEvent &= ~removedEvents;
        if(detachedSocketEvent == 0)
          detachedSockInfo = 0;
      }
    }
  }
  uint addedEvents = events & ~sockInfo->events;
  if(addedEvents)
  {
    if(addedEvents & (readFlag | writeFlag))
    {
      if (!sockInfo->completionPortCreated)
      {
        VERIFY(CreateIoCompletionPort((HANDLE)socket.s, completionPort, sockInfo->key, 0) == completionPort);
        sockInfo->completionPortCreated = true;
      }
      if(addedEvents & readFlag)
      {
        WSABUF buf = {};
        DWORD flags = MSG_PEEK;
        WSARecv((SOCKET)socket.s, &buf, 1, NULL, &flags, &readOverlapped, NULL);
      }
      if(addedEvents & writeFlag)
      {
        WSABUF buf = {};
        WSASend((SOCKET)socket.s, &buf, 1, NULL, 0, &writeOverlapped, NULL);
      }
    }
    if(addedEvents & connectFlag)
    {
      Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addConnect, sockInfo->s, sockInfo->key};
      pushWaitThreadMessage(addMessage);
    }
    if(addedEvents & acceptFlag)
    {
      Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addAccept, sockInfo->s, sockInfo->key};
      pushWaitThreadMessage(addMessage);
    }
  }
  sockInfo->events = events;
}

void Socket::Poll::Private::pushWaitThreadMessage(const WaitThreadMessage& message)
{
  EnterCriticalSection(&waitThreadMutex);
  if(!waitThread)
  {
    waitThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(!waitThreadEvent)
    {
      LeaveCriticalSection(&waitThreadMutex);
      return;
    }
    waitThread = CreateThread(0, 0, (unsigned long (__stdcall*)(void*))waitThreadProc, this, 0, 0);
    if(!waitThread)
    {
      CloseHandle(waitThreadEvent);
      waitThreadEvent = 0;
      LeaveCriticalSection(&waitThreadMutex);
      return;
    }
  }
  else
    SetEvent(waitThreadEvent);
  waitThreadQueue.append(message);
  LeaveCriticalSection(&waitThreadMutex);
}

void Socket::Poll::Private::joinWaitThread()
{
  if(waitThread)
  {
    HANDLE thread = 0;
    EnterCriticalSection(&waitThreadMutex);
    if(waitThread)
    {
      thread = waitThread;
      waitThread = 0;
      WaitThreadMessage quitMessage = {WaitThreadMessage::quit};
      waitThreadQueue.prepend(quitMessage);
      SetEvent(waitThreadEvent);
    }
    LeaveCriticalSection(&waitThreadMutex);
    if(thread)
    {
      WaitForSingleObject(thread, INFINITE);
      CloseHandle(thread);
    }
  }
}

uint Socket::Poll::Private::waitThreadProc(Private* p)
{
  struct EventData
  {
    ULONG_PTR key;
    LPOVERLAPPED overlapped;
    SOCKET s;
  };

  HANDLE events[64];
  HashMap<HANDLE, EventData> eventData;
  Map<SOCKET, HANDLE> suspendedEvents;
  DWORD eventCount = 1;
  events[0] = p->waitThreadEvent;

  for(WaitThreadMessage message;;)
  {
    EnterCriticalSection(&p->waitThreadMutex);
    if(p->waitThreadQueue.isEmpty())
    {
      LeaveCriticalSection(&p->waitThreadMutex);
      goto skipInputMessage;
    }
    message = p->waitThreadQueue.front();
    switch(message.type)
    {
    case WaitThreadMessage::quit:
      p->waitThreadQueue.removeFront();
      goto cleanup;
    case WaitThreadMessage::addConnect:
    case WaitThreadMessage::addAccept:
      if(eventCount >= 64)
      {
        LeaveCriticalSection(&p->waitThreadMutex);
        goto skipInputMessage;
      }
    default:
      p->waitThreadQueue.removeFront();
      break;
    }
    LeaveCriticalSection(&p->waitThreadMutex);

    switch(message.type)
    {
    case WaitThreadMessage::addConnect:
    case WaitThreadMessage::addAccept:
      {
        WSAEVENT eventHandle;
        Map<SOCKET, HANDLE>::Iterator it = suspendedEvents.find(message.s);
        if(it == suspendedEvents.end())
        {
          eventHandle = WSACreateEvent();
          if(eventHandle != WSA_INVALID_EVENT)
          {
            if(WSAEventSelect(message.s, eventHandle, message.type == WaitThreadMessage::addConnect ? (FD_CONNECT | FD_CLOSE) : FD_ACCEPT) == SOCKET_ERROR)
            {
              WSACloseEvent(eventHandle);
              eventHandle = WSA_INVALID_EVENT;
            }
          }
        }
        else
        {
          eventHandle = *it;
          suspendedEvents.remove(it);
        }
        if(eventHandle != WSA_INVALID_EVENT)
        {
          events[eventCount++] = eventHandle;
          EventData& event = eventData.append(eventHandle, EventData());
          event.key = message.key;
          event.overlapped = message.type == WaitThreadMessage::addConnect ? &p->connectOverlapped : &p->acceptOverlapped;
          event.s = message.s;
        }
      }
      break;
    case WaitThreadMessage::remove:
      {
        Map<SOCKET, HANDLE>::Iterator it = suspendedEvents.find(message.s);
        if(it == suspendedEvents.end())
        {
          for(DWORD i = 0; i < eventCount; ++i)
          {
            HANDLE eventHandle = events[i];
            HashMap<HANDLE, EventData>::Iterator it = eventData.find(eventHandle);
            EventData& event = *it;
            if(event.s == message.s)
            {
              --eventCount;
              for(DWORD nextEventNum; i < eventCount; i = nextEventNum)
              {
                nextEventNum = i + 1;
                events[i] = events[nextEventNum];
              }
              eventData.remove(it);
              break;
            }
          }
        }
        else
        {
          WSACloseEvent(*it);
          suspendedEvents.remove(it);
        }
        break;
      }
    }
    if(eventCount == 1 && suspendedEvents.isEmpty())
    {
      EnterCriticalSection(&p->waitThreadMutex);
      if(p->waitThreadQueue.isEmpty())
        goto cleanup;
    }
    continue;

  skipInputMessage:
    DWORD eventNum = WaitForMultipleObjects(eventCount, events, FALSE, INFINITE);
    if(eventNum > WAIT_OBJECT_0)
    {
      HANDLE eventHandle = events[eventNum - WAIT_OBJECT_0];
      HashMap<HANDLE, EventData>::Iterator it = eventData.find(eventHandle);
      EventData& event = *it;

      --eventCount;
      for(DWORD nextEventNum; eventNum < eventCount; eventNum = nextEventNum)
      {
        nextEventNum = eventNum + 1;
        events[eventNum] = events[nextEventNum];
      }

      if(event.overlapped == &p->connectOverlapped)
      {
        WSACloseEvent(eventHandle);
        PostQueuedCompletionStatus(p->completionPort, 0, event.key, event.overlapped);
        eventData.remove(it);
        if(eventCount == 1 && suspendedEvents.isEmpty())
        {
          EnterCriticalSection(&p->waitThreadMutex);
          if(p->waitThreadQueue.isEmpty())
            goto cleanup;
        }
      }
      else
      {
        WSANETWORKEVENTS selectedEvents = {};
        WSAEnumNetworkEvents(event.s, eventHandle, &selectedEvents);
        suspendedEvents.insert(event.s, eventHandle);
        PostQueuedCompletionStatus(p->completionPort, 0, event.key, event.overlapped);
        eventData.remove(it);
      }
    }
    else if(eventNum == WAIT_OBJECT_0)
      ResetEvent(events[0]);
  }

cleanup:
  WSACloseEvent(p->waitThreadEvent);
  p->waitThreadEvent = 0;
  if(p->waitThread)
  {
    CloseHandle(p->waitThread);
    p->waitThread = 0;
  }
  LeaveCriticalSection(&p->waitThreadMutex);
  for(DWORD i = 1; i < eventCount; ++i)
    WSACloseEvent(events[i]);
  for(Map<SOCKET, HANDLE>::Iterator i = suspendedEvents.begin(), end = suspendedEvents.end(); i != end; ++i)
    WSACloseEvent(*i);
  return 0;
}

void Socket::Poll::Private::remove(Socket& socket)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it == sockets.end())
    return;
  Private::SocketInfo& sockInfo = *it;
  if(sockInfo.events & (connectFlag | acceptFlag))
  {
    Private::WaitThreadMessage message = {Private::WaitThreadMessage::remove, sockInfo.s, sockInfo.key};
    pushWaitThreadMessage(message);
  }
  if(detachedSockInfo == &sockInfo)
    detachedSockInfo = 0;
  keys.remove(sockInfo.key);
  sockets.remove(it);
}

bool Socket::Poll::Private::poll(Event& event, int64 timeout)
{
  if(detachedSockInfo)
  {
    switch(detachedSocketEvent)
    {
    case readFlag:
      {
        WSABUF buf = {};
        DWORD flags = MSG_PEEK;
        WSARecv(detachedSockInfo->s, &buf, 1, NULL, &flags, &readOverlapped, NULL);
      }
      break;
    case writeFlag:
      {
        WSABUF buf = {};
        WSASend(detachedSockInfo->s, &buf, 1, NULL, 0, &writeOverlapped, NULL);
      }
      break;
    case acceptFlag:
      {
        Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addAccept, detachedSockInfo->s, detachedSockInfo->key};
        pushWaitThreadMessage(addMessage);
      }
      break;
    }
    detachedSockInfo = 0;
  }
  DWORD numberOfBytes;
  ULONG_PTR completionKey = 0;
  Private::Overlapped* overlapped;
  for(;;)
  {
    if(!GetQueuedCompletionStatus(completionPort, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapped, (DWORD)timeout))
    {
      if(!overlapped)
        switch(GetLastError())
        {
        case WAIT_TIMEOUT:
          event.flags = 0;
          event.socket = 0;
          return true;
        default:
          return false;
        }
    }
    HashMap<ULONG_PTR, Private::SocketInfo*>::Iterator it =  keys.find(completionKey);
    if(it == keys.end())
    {
      if(!overlapped->event)
      {
        event.flags = 0;
        event.socket = 0;
        return true; // interrupt
      }
      continue;
    }
    Private::SocketInfo* sockInfo = *it;
    if(!(sockInfo->events & overlapped->event))
      continue;
    event.socket = sockInfo->socket;
    event.flags = overlapped->event;
    detachedSockInfo = sockInfo;
    detachedSocketEvent = overlapped->event;
    return true;
  }
}

bool Socket::Poll::Private::interrupt()
{
  return PostQueuedCompletionStatus(completionPort, 0, 0, &interruptOverlapped) == TRUE;
}

#elif defined(__linux__)

class Socket::Poll::Private
{
public:
  Private();
  ~Private();

  inline void set(Socket& socket, uint events);
  inline void remove(Socket& socket);
  inline void clear();
  inline bool poll(Event& event, int64 timeout);
  inline bool interrupt();

private:
  struct SocketInfo
  {
    Socket* socket;
    uint events;
    SOCKET s;
  };

private:
  int fd;
  int eventFd;
  HashMap<Socket*, SocketInfo> sockets;
  HashMap<Socket*, uint> selectedSockets;

  inline static uint32 mapEvents(uint events);
  inline static uint unmapEvents(uint32 nativeEvents, uint events);
};

Socket::Poll::Private::Private()
{
  fd = epoll_create1(EPOLL_CLOEXEC);
  eventFd = eventfd(0, EFD_CLOEXEC);
  epoll_event ev;
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
  ev.data.ptr = NULL;
  VERIFY(epoll_ctl(fd, EPOLL_CTL_ADD, eventFd, &ev) == 0);
}

Socket::Poll::Private::~Private()
{
  ::close(fd);
  ::close(eventFd);
}

uint32 Socket::Poll::Private::mapEvents(uint events)
{
  uint32 pollEvents = 0;
  if(events & (readFlag | acceptFlag))
    pollEvents = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
  if(events & (writeFlag | connectFlag))
    pollEvents |= EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
  return pollEvents;
}

uint Socket::Poll::Private::unmapEvents(uint32 nativeEvents, uint events)
{
  uint result = 0;
  if(nativeEvents & (EPOLLIN | EPOLLRDHUP | EPOLLHUP))
    result = events & (readFlag | acceptFlag);
  if(nativeEvents & EPOLLOUT || (result == 0 && nativeEvents & (EPOLLRDHUP | EPOLLHUP)))
    result |= events & (writeFlag | connectFlag);
  return result;
}

void Socket::Poll::Private::set(Socket& socket, uint events)
{
  epoll_event ev;
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it != sockets.end())
  {
    Private::SocketInfo& sockInfo = *it;
    if(sockInfo.events == events)
      return;
    uint removedEvents = sockInfo.events & ~events;
    sockInfo.events = events;
    ev.events = mapEvents(events);
    ev.data.ptr = &sockInfo;
    VERIFY(epoll_ctl(fd, EPOLL_CTL_MOD, socket.s, &ev) == 0);
    HashMap<Socket*, uint>::Iterator it = selectedSockets.find(sockInfo.socket);
    if(it != selectedSockets.end())
    {
      uint& selectedEvents = *it;
      selectedEvents &= ~removedEvents;
      if(selectedEvents == 0)
        selectedSockets.remove(it);
    }
  }
  else
  {
    SOCKET s = socket.s;
    if(s == INVALID_SOCKET)
      return;
    Private::SocketInfo& sockInfo = sockets.append(&socket, Private::SocketInfo());
    sockInfo.s = s;
    sockInfo.socket = &socket;
    sockInfo.events = events;
    ev.events = mapEvents(events);
    ev.data.ptr = &sockInfo;
    VERIFY(epoll_ctl(fd, EPOLL_CTL_ADD, s, &ev) == 0);
  }
}

void Socket::Poll::Private::remove(Socket& socket)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it == sockets.end())
    return;
  Private::SocketInfo& sockInfo = *it;
  epoll_event ev;
  VERIFY(epoll_ctl(fd, EPOLL_CTL_DEL, sockInfo.s, &ev) == 0);
  sockets.remove(it);
  selectedSockets.remove(&socket);
}

void Socket::Poll::Private::clear()
{
  ::close(fd);
  fd = epoll_create1(EPOLL_CLOEXEC);
  sockets.clear();
  selectedSockets.clear();
  epoll_event ev;
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
  ev.data.ptr = NULL;
  VERIFY(epoll_ctl(fd, EPOLL_CTL_ADD, eventFd, &ev) == 0);
}

bool Socket::Poll::Private::poll(Event& event, int64 timeout)
{
  if(selectedSockets.isEmpty())
  {
    epoll_event events[64];
    int count = ::epoll_wait(fd, events, sizeof(events) / sizeof(*events), timeout);
    bool interrupted = false;
    for(epoll_event* i = events, * end = events + count; i < end; ++i)
    {
      Private::SocketInfo* sockInfo = (Private::SocketInfo*)i->data.ptr;
      if(sockInfo)
        selectedSockets.append(sockInfo->socket, unmapEvents(i->events, sockInfo->events));
      else if(i->events & EPOLLIN)
        interrupted = true;
    }

    if(selectedSockets.isEmpty() || interrupted)
    {
      if(interrupted)
      {
        uint64 val;
        VERIFY(read(eventFd, &val, sizeof(val)) == sizeof(val));
      }
      event.flags = 0;
      event.socket = 0;
      return true; // timeout or interrupt
    }
  }

  HashMap<Socket*, uint>::Iterator it = selectedSockets.begin();
  event.socket = it.key();
  event.flags = *it;
  selectedSockets.remove(it);
  return true;
}

bool Socket::Poll::Private::interrupt()
{
  uint64 val = 1;
  return write(eventFd, &val, sizeof(val)) != -1;
}

#else

class Socket::Poll::Private
{
public:
  Private();
  ~Private();

  inline void clear();
  inline void set(Socket& socket, uint events);
  inline void remove(Socket& socket);
  inline bool poll(Event& event, int64 timeout);
  inline bool interrupt();

private:
  struct SocketInfo
  {
    Socket* socket;
    usize index;
    uint events;
  };

private:
  int interruptFd;
  Array<pollfd> pollfds;
  HashMap<Socket*, SocketInfo> sockets;
  HashMap<SOCKET, SocketInfo*> fdToSocket;
  HashMap<Socket*, uint> selectedSockets;

  inline static short mapEvents(uint events);
  inline static uint unmapEvents(short nativeEvents, uint events);
};

Socket::Poll::Private::Private()
{
  pollfd& pfd = pollfds.append(pollfd());
  pfd.fd = interruptFd = eventfd(0, EFD_CLOEXEC);
  pfd.events = POLLIN | POLLRDHUP | POLLHUP;
  pfd.revents = 0;
}

Socket::Poll::Private::~Private()
{
  ::close(interruptFd);
}

short Socket::Poll::Private::mapEvents(uint events)
{
  short pollEvents = 0;
  if(events & (readFlag | acceptFlag))
    pollEvents = POLLIN | POLLRDHUP | POLLHUP;
  if(events & (writeFlag | connectFlag))
    pollEvents |= POLLOUT | POLLRDHUP | POLLHUP;
  return pollEvents;
}

uint Socket::Poll::Private::unmapEvents(short nativeEvents, uint events)
{
  uint result = 0;
  if(nativeEvents & (POLLIN | POLLRDHUP | POLLHUP))
    result = events & (readFlag | acceptFlag);
  if(nativeEvents & POLLOUT || (result == 0 && nativeEvents & (POLLRDHUP | POLLHUP)))
    result |= events & (writeFlag | connectFlag);
  return result;
}

void Socket::Poll::Private::clear()
{
  pollfds.resize(1);
  sockets.clear();
  fdToSocket.clear();
  selectedSockets.clear();
}

void Socket::Poll::Private::set(Socket& socket, uint events)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it != sockets.end())
  {
    Private::SocketInfo& sockInfo = *it;
    if(sockInfo.events == events)
      return;
    uint removedEvents = sockInfo.events & ~events;
    pollfds[sockInfo.index].events = Private::mapEvents(events);
    sockInfo.events = events;
    HashMap<Socket*, uint>::Iterator it = selectedSockets.find(sockInfo.socket);
    if(it != selectedSockets.end())
    {
      uint& selectedEvents = *it;
      selectedEvents &= ~removedEvents;
      if(selectedEvents == 0)
        selectedSockets.remove(it);
    }
  }
  else
  {
    SOCKET s = socket.s;
    if(s == INVALID_SOCKET)
      return;
    Private::SocketInfo& sockInfo = sockets.append(&socket, Private::SocketInfo());
    sockInfo.socket = &socket;
    sockInfo.index = pollfds.size();
    sockInfo.events = events;
    fdToSocket.append(s, &sockInfo);
    pollfd& pfd = pollfds.append(pollfd());
    pfd.fd = socket.s;
    pfd.events = Private::mapEvents(events);
    pfd.revents = 0;
  }
}

void Socket::Poll::Private::remove(Socket& socket)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = sockets.find(&socket);
  if(it == sockets.end())
    return;
  Private::SocketInfo& sockInfo = *it;
  usize index = sockInfo.index;
  pollfd& pfd = pollfds[index];
  fdToSocket.remove(pfd.fd);
  pollfds.remove(index);
  sockets.remove(it);
  for(HashMap<Socket*, Private::SocketInfo>::Iterator i = sockets.begin(), end = sockets.end(); i != end; ++i)
  {
    Private::SocketInfo& sockInfo = *i;
    if(sockInfo.index > index)
      --sockInfo.index;
  }
  selectedSockets.remove(&socket);
}

bool Socket::Poll::Private::poll(Event& event, int64 timeout)
{
  if(selectedSockets.isEmpty())
  {
    int count = ::poll(pollfds, pollfds.size(), timeout);
    if(count > 0)
    {
      for(pollfd* i = pollfds + 1, * end = i + pollfds.size(); i < end; ++i)
      {
        if(i->revents)
        {
          HashMap<SOCKET, Private::SocketInfo*>::Iterator it = fdToSocket.find(i->fd);
          ASSERT(it != fdToSocket.end());
          Private::SocketInfo* sockInfo = *it;
          selectedSockets.append(sockInfo->socket, unmapEvents(i->revents, sockInfo->events));
          i->revents = 0;
          if(--count == 0)
            break;
        }
      }
    }

    bool interrupted;;
    if(selectedSockets.isEmpty() || (interrupted = pollfds[0].revents & POLLIN))
    {
      if(interrupted)
      {
        uint64 val;
        read(interruptFd, &val, sizeof(val));
        pollfds[0].revents = 0;
      }
      event.flags = 0;
      event.socket = 0;
      return true; // timeout or interrupt
    }
  }

  HashMap<Socket*, uint>::Iterator it = selectedSockets.begin();
  event.socket = it.key();
  event.flags = *it;
  selectedSockets.remove(it);
  return true;
}

bool Socket::Poll::Private::interrupt()
{
  uint64 val = 1;
  return write(interruptFd, &val, sizeof(val)) != -1;
}

#endif

Socket::Poll::Poll() : p(new Private) {}
Socket::Poll::~Poll() {delete p;}
void Socket::Poll::set(Socket& socket, uint flags) {return p->set(socket, flags);}
void Socket::Poll::remove(Socket& socket) {return p->remove(socket);}
void Socket::Poll::clear() {return p->clear();}
bool Socket::Poll::poll(Event& event, int64 timeout) {return p->poll(event, timeout);}
bool Socket::Poll::interrupt() {return p->interrupt();}
