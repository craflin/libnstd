
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
#include <errno.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <cstring>
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

#include <nstd/Debug.h>
#include <nstd/HashMap.h>
#include <nstd/Array.h>
#include <nstd/Socket/Socket.h>
#ifdef _WIN32
#include <nstd/List.h>
#include <nstd/Map.h>
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
  DWORD val = enable ? 1 : 0;
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
  in_addr in;
  in.s_addr = htonl(ip);
#ifdef UNICODE
  WCHAR buf[17];
  buf[0] = _T('\0');
  RtlIpv4AddressToString(&in, buf);
#else
  char* buf = inet_ntoa(in);
#endif
  return String(buf, String::length(buf));
}

class Socket::Poll::Private
{
public:
#ifdef _WIN32
  HANDLE completionPort;

  struct Overlapped : public WSAOVERLAPPED
  {
    uint event;
  };

  struct SocketInfo
  {
    Socket* socket;
    ULONG_PTR key;
    uint events;
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

  Private() : nextKey(1), detachedSockInfo(0), waitThread(0), waitThreadEvent(0)
  {
    InitializeCriticalSection(&waitThreadMutex);
    ZeroMemory(&readOverlapped, sizeof(readOverlapped));
    ZeroMemory(&writeOverlapped, sizeof(writeOverlapped));
    connectOverlapped.event = connectFlag;
    acceptOverlapped.event = acceptFlag;
    readOverlapped.event = readFlag;
    writeOverlapped.event = writeFlag;
    interruptOverlapped.event = 0;
  }

  ~Private()
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
    DeleteCriticalSection(&waitThreadMutex);
  }

  static uint waitThreadProc(Private* p);

  void pushWaitThreadMessage(const WaitThreadMessage& message);

#else
  struct SocketInfo
  {
    Socket* socket;
    usize index;
    uint events;
  };

  Array<pollfd> pollfds;
  HashMap<Socket*, SocketInfo> sockets;
  HashMap<SOCKET, SocketInfo*> fdToSocket;
  HashMap<Socket*, uint> selectedSockets;

  static short mapEvents(uint events)
  {
    short pollEvents = 0;
    if(events & (readFlag | acceptFlag))
      pollEvents |= POLLIN | POLLRDHUP | POLLHUP;
    if(events & (writeFlag | connectFlag))
      pollEvents |= POLLOUT | POLLRDHUP | POLLHUP;
    return pollEvents;
  }
#endif
};

Socket::Poll::Poll() : p(new Private)
{
#ifdef _WIN32
  p->completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
#else
  pollfd& pfd = p->pollfds.append(pollfd());
  pfd.fd = eventfd(0, EFD_CLOEXEC);
  pfd.events = POLLIN | POLLRDHUP | POLLHUP;
  pfd.revents = 0;
#endif
}

Socket::Poll::~Poll()
{
#ifdef _WIN32
  if(p->completionPort)
    CloseHandle(p->completionPort);
#else
  ::close(p->pollfds[0].fd);
#endif
  delete p;
}

void Socket::Poll::clear()
{
#ifdef _WIN32
  // todo
#else
  p->pollfds.resize(1);
  p->sockets.clear();
  p->fdToSocket.clear();
  p->selectedSockets.clear();
#endif
}

void Socket::Poll::set(Socket& socket, uint events)
{
#ifdef _WIN32
  Private::SocketInfo* sockInfo;
  HashMap<Socket*, Private::SocketInfo>::Iterator it = p->sockets.find(&socket);
  if(it == p->sockets.end()) // this is a new one, lets create a key for it and attach it to the completion port
  {
    sockInfo = &p->sockets.append(&socket, Private::SocketInfo());
    sockInfo->key = p->nextKey++;
    sockInfo->socket = &socket;
    sockInfo->events = 0;
    VERIFY(CreateIoCompletionPort((HANDLE)socket.s, p->completionPort, sockInfo->key, 0) == p->completionPort);
    p->keys.append(sockInfo->key, sockInfo);
  }
  else
  {
    sockInfo = &*it;
    uint removedEvents = sockInfo->events & ~events;
    if(removedEvents & (connectFlag | acceptFlag))
    {
      Private::WaitThreadMessage removeMessage = {Private::WaitThreadMessage::remove, socket.s, sockInfo->key};
      p->pushWaitThreadMessage(removeMessage);
    }
    if(p->detachedSockInfo == sockInfo && removedEvents & p->detachedSocketEvent)
      p->detachedSockInfo = 0;
  }
  uint addedEvents = events & ~sockInfo->events;
  if(addedEvents & readFlag)
  {
    WSABUF buf = {};
    DWORD flags = MSG_PEEK;
    WSARecv((SOCKET)socket.s, &buf, 1, NULL, &flags, &p->readOverlapped, NULL);
  }
  if(addedEvents & writeFlag)
  {
    WSABUF buf = {};
    WSASend((SOCKET)socket.s, &buf, 1, NULL, 0, &p->writeOverlapped, NULL);
  }
  if(addedEvents & connectFlag)
  {
    Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addConnect, socket.s, sockInfo->key};
    p->pushWaitThreadMessage(addMessage);
  }
  if(addedEvents & acceptFlag)
  {
    Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addAccept, socket.s, sockInfo->key};
    p->pushWaitThreadMessage(addMessage);
  }

  sockInfo->events = events;
#else
  HashMap<Socket*, Private::SocketInfo>::Iterator it = p->sockets.find(&socket);
  if(it != p->sockets.end())
  {
    Private::SocketInfo& sockInfo = *it;
    if(sockInfo.events == events)
      return;
    p->pollfds[sockInfo.index].events = Private::mapEvents(events);
    sockInfo.events = events;
  }
  else
  {
    SOCKET s = socket.s;
    if(s == INVALID_SOCKET)
      return;
    Private::SocketInfo& sockInfo = p->sockets.append(&socket, Private::SocketInfo());
    sockInfo.socket = &socket;
    sockInfo.index = p->pollfds.size();
    sockInfo.events = events;
    p->fdToSocket.append(s, &sockInfo);
    pollfd& pfd = p->pollfds.append(pollfd());
    pfd.fd = socket.s;
    pfd.events = Private::mapEvents(events);
    pfd.revents = 0;
  }
#endif
}

#ifdef _WIN32
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
#endif

void Socket::Poll::remove(Socket& socket)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = p->sockets.find(&socket);
  if(it == p->sockets.end())
    return;
  Private::SocketInfo& sockInfo = *it;
#ifdef _WIN32
  if(sockInfo.events & (connectFlag | acceptFlag))
  {
    Private::WaitThreadMessage message = {Private::WaitThreadMessage::remove, socket.s, sockInfo.key};
    p->pushWaitThreadMessage(message);
  }
  if(p->detachedSockInfo == &sockInfo)
    p->detachedSockInfo = 0;
  p->keys.remove(sockInfo.key);
  p->sockets.remove(it);
  // todo: detach socket from iocp?
#else
  usize index = sockInfo.index;
  pollfd& pfd = p->pollfds[index];
  p->fdToSocket.remove(pfd.fd);
  p->pollfds.remove(index);
  p->sockets.remove(it);
  for(HashMap<Socket*, Private::SocketInfo>::Iterator i = p->sockets.begin(), end = p->sockets.end(); i != end; ++i)
  {
    Private::SocketInfo& sockInfo = *i;
    if(sockInfo.index > index)
      --sockInfo.index;
  }
  p->selectedSockets.remove(&socket);
#endif
}

bool Socket::Poll::poll(Event& event, int64 timeout)
{
#ifdef _WIN32
  if(p->detachedSockInfo)
  {
    switch(p->detachedSocketEvent)
    {
    case readFlag:
      {
        WSABUF buf = {};
        DWORD flags = MSG_PEEK;
        WSARecv((SOCKET)p->detachedSockInfo->socket->s, &buf, 1, NULL, &flags, &p->readOverlapped, NULL);
      }
      break;
    case writeFlag:
      {
        WSABUF buf = {};
        WSASend((SOCKET)p->detachedSockInfo->socket->s, &buf, 1, NULL, 0, &p->writeOverlapped, NULL);
      }
      break;
    case acceptFlag:
      {
        Private::WaitThreadMessage addMessage = {Private::WaitThreadMessage::addAccept, (SOCKET)p->detachedSockInfo->socket->s, p->detachedSockInfo->key };
        p->pushWaitThreadMessage(addMessage);
      }
      break;
    }
    p->detachedSockInfo = 0;
  }
  DWORD numberOfBytes;
  ULONG_PTR completionKey = 0;
  Private::Overlapped* overlapped;
  for(;;)
  {
    if(!GetQueuedCompletionStatus(p->completionPort, &numberOfBytes, &completionKey, (LPOVERLAPPED*)&overlapped, (DWORD)timeout))
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
    HashMap<ULONG_PTR, Private::SocketInfo*>::Iterator it =  p->keys.find(completionKey);
    if(it == p->keys.end())
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
    p->detachedSockInfo = sockInfo;
    p->detachedSocketEvent = overlapped->event;
    return true;
  }
#else
  if(p->selectedSockets.isEmpty())
  {
    int count = ::poll(p->pollfds, p->pollfds.size(), timeout);
    if(count > 0)
    {
      for(pollfd* i = p->pollfds + 1, * end = i + p->pollfds.size(); i < end; ++i)
      {
        if(i->revents)
        {
          HashMap<SOCKET, Private::SocketInfo*>::Iterator it = p->fdToSocket.find(i->fd);
          ASSERT(it != p->fdToSocket.end());
          Private::SocketInfo* sockInfo = *it;
          uint events = 0;
          int revents = i->revents;

          if(revents & (POLLIN | POLLRDHUP | POLLHUP))
            events |= sockInfo->events & (readFlag | acceptFlag);
          if(revents & POLLOUT || (events == 0 && revents & (POLLRDHUP | POLLHUP)))
            events |= sockInfo->events & (writeFlag | connectFlag);

          p->selectedSockets.append(sockInfo->socket, events);

          i->revents = 0;
          if(--count == 0)
            break;
        }
      }
    }

    if(p->selectedSockets.isEmpty())
    {
      if(p->pollfds[0].revents)
      {
        uint64 val;
        read(p->pollfds[0].fd, &val, sizeof(val));
        p->pollfds[0].revents = 0;
      }
      event.flags = 0;
      event.socket = 0;
      return true; // timeout or interrupt
    }
  }

  HashMap<Socket*, uint>::Iterator it = p->selectedSockets.begin();
  event.socket = it.key();
  event.flags = *it;
  p->selectedSockets.remove(it);
  return true;
#endif
}

bool Socket::Poll::interrupt()
{
#ifdef _WIN32
  return PostQueuedCompletionStatus(p->completionPort, 0, 0, &p->interruptOverlapped) == TRUE;
#else
  uint64 val = 1;
  return write(p->pollfds[0].fd, &val, sizeof(val)) != -1;
#endif
}
