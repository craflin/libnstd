
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS // todo: get rid of this
#define _CRT_NO_POSIX_ERROR_CODES
#include <winsock2.h>
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
};

#ifdef _WIN32
Socket::Private::Winsock Socket::Private::winsock;
#endif

Socket::Socket() : s(INVALID_SOCKET) {}

Socket::~Socket()
{
  if(s != INVALID_SOCKET)
    ::CLOSE(s);
}

bool Socket::open()
{
  if(s != INVALID_SOCKET)
    close();

#ifdef _WIN32
  s = socket(AF_INET, SOCK_STREAM, 0);
#else
  s = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
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
    serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    conn = socket(AF_INET, SOCK_STREAM, 0);
    if(serv == INVALID_SOCKET)
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
  if(ioctlsocket(s, FIONBIO, &val))
#else
  if(fcntl(s, F_SETFL, O_NONBLOCK))
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

bool Socket::bind(unsigned int ip, unsigned short port)
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

bool Socket::connect(unsigned int ip, unsigned short port)
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
    return RtlIpv4StringToAddress(addr, FALSE, &end, &inaddr);
#else
    return ntohl(inet_addr((const tchar*)addr.substr(0, (portStr - (const tchar*)addr) / sizeof(tchar))));
#endif
  }
#ifdef UNICODE
  in_addr inaddr;
  LPCWSTR end; 
  return RtlIpv4StringToAddress(addr, FALSE, &end, &inaddr);
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
  struct SocketInfo
  {
    Socket* socket;
    SOCKET s;
    WSAEVENT event;
    uint events;
  };
  Array<WSAEVENT> events;
  HashMap<Socket*, SocketInfo> sockets;
  HashMap<WSAEVENT, SocketInfo*> eventToSocket;

  static long mapEvents(uint events)
  {
    long networkEvents = 0;
    if(events & readFlag)
      networkEvents |= FD_READ | FD_CLOSE;
    if(events & writeFlag)
      networkEvents |= FD_WRITE | FD_CLOSE;
    if(events & acceptFlag)
      networkEvents |= FD_ACCEPT;
    if(events & connectFlag)
      networkEvents |= FD_CONNECT | FD_CLOSE;
    return networkEvents;
  }
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
  p->events.append(WSACreateEvent());
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
  WSACloseEvent(p->events[0]);
#else
  ::close(p->pollfds[0].fd);
#endif
  delete p;
}

void Socket::Poll::clear()
{
#ifdef _WIN32
  p->events.resize(1);
  for(HashMap<Socket*, Private::SocketInfo>::Iterator i = p->sockets.begin(), end = p->sockets.end(); i != end; ++i)
    WSACloseEvent(i->event);
  p->sockets.clear();
  p->eventToSocket.clear();
#else
  p->pollfds.resize(1);
  p->sockets.clear();
  p->fdToSocket.clear();
  p->selectedSockets.clear();
#endif
}

void Socket::Poll::set(Socket& socket, uint events)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = p->sockets.find(&socket);
#ifdef _WIN32
  if(it != p->sockets.end())
  {
    Private::SocketInfo& sockInfo = *it;
    if(sockInfo.events == events)
      return;
    long networkEvents = Private::mapEvents(events);
    VERIFY(WSAEventSelect(sockInfo.s, sockInfo.event, networkEvents) !=  SOCKET_ERROR);
    sockInfo.events = events;
  }
  else
  {
    SOCKET s = socket.s;
    if(s == INVALID_SOCKET)
      return;
    Private::SocketInfo& sockInfo = p->sockets.append(&socket, Private::SocketInfo());
    sockInfo.socket = &socket;
    sockInfo.s = s;
    WSAEVENT event = WSACreateEvent();
    ASSERT(event != WSA_INVALID_EVENT);
    sockInfo.event = event;
    long networkEvents = Private::mapEvents(events);
    VERIFY(WSAEventSelect(s, event, networkEvents) !=  SOCKET_ERROR);
    sockInfo.events = events;
    p->events.append(event);
    p->eventToSocket.append(event, &sockInfo);
  }
#else
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

void Socket::Poll::remove(Socket& socket)
{
  HashMap<Socket*, Private::SocketInfo>::Iterator it = p->sockets.find(&socket);
  if(it == p->sockets.end())
    return;
  Private::SocketInfo& sockInfo = *it;
#ifdef _WIN32
  WSAEVENT event = sockInfo.event;
  Array<WSAEVENT>::Iterator itEvent = p->events.find(event);
  if(itEvent != p->events.end())
    p->events.remove(itEvent);
  WSACloseEvent(event);
  p->sockets.remove(it);
  p->eventToSocket.remove(event);
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
  DWORD count = (DWORD)p->events.size();
  DWORD dw = WSAWaitForMultipleEvents(count, (WSAEVENT*)p->events, FALSE, (DWORD)timeout, FALSE);
  if(dw >= WSA_WAIT_EVENT_0 + 1 && dw < WSA_WAIT_EVENT_0 + count)
  {
    WSAEVENT wsaEvent = p->events[dw - WSA_WAIT_EVENT_0];
    HashMap<WSAEVENT, Private::SocketInfo*>::Iterator it = p->eventToSocket.find(wsaEvent);
    ASSERT(it != p->eventToSocket.end());
    Private::SocketInfo* sockInfo = *it;
    WSANETWORKEVENTS selectedEvents = {};
    VERIFY(WSAEnumNetworkEvents(sockInfo->s, wsaEvent, &selectedEvents) != SOCKET_ERROR);
    uint32 revents = selectedEvents.lNetworkEvents;

    event.flags = 0;
    if(revents & (FD_READ | FD_CLOSE | FD_ACCEPT))
      event.flags |= sockInfo->events & (readFlag | acceptFlag);
    if(revents & (FD_WRITE | FD_CONNECT) || (event.flags == 0 && revents & FD_CLOSE))
      event.flags |= sockInfo->events & (writeFlag | connectFlag);

    event.socket = sockInfo->socket;
    return true;
  }
  else
  {
    if(dw == WSA_WAIT_EVENT_0)
      WSAResetEvent(p->events[0]);
    event.flags = 0;
    event.socket = 0;
    return true; // timeout or interrupt
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
  return WSASetEvent(p->events[0]) == TRUE;
#else
  uint64 val = 1;
  return write(p->pollfds[0].fd, &val, sizeof(val)) != -1;
#endif
}
