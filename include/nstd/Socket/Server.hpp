
#pragma once

#include <nstd/String.hpp>

class Socket;

class Server
{
public:
  class Client
  {
  public:
    class ICallback
    {
    public:
      virtual void onRead() = 0;
      virtual void onWrite() = 0;
      virtual void onClosed() = 0;

    protected:
      ICallback() {}
      ~ICallback() {}
    };

  public:
    bool write(const byte *data, usize size, usize *postponed = 0);
    bool read(byte *buffer, usize maxSize, usize &size);
    void suspend();
    void resume();
    Socket& getSocket();

  private:
    Client(const Client &);
    Client &operator=(const Client &);
    Client();
    ~Client();
  };

  class Listener
  {
  public:
    class ICallback
    {
    public:
      virtual Client::ICallback *onAccepted(Client &client, uint32 ip, uint16 port) = 0;

    protected:
      ICallback() {}
      ~ICallback() {}
    };

  private:
    Listener(const Listener &);
    Listener &operator=(const Listener &);
    Listener();
    ~Listener();
  };

  class Establisher
  {
  public:
    class ICallback
    {
    public:
      virtual Client::ICallback *onConnected(Client &client) = 0;
      virtual void onAbolished() = 0;

    protected:
      ICallback() {}
      ~ICallback() {}
    };

  private:
    Establisher(const Establisher &);
    Establisher &operator=(const Establisher &);
    Establisher();
    ~Establisher();
  };

  class Timer
  {
  public:
    class ICallback
    {
    public:
      virtual void onActivated() = 0;

    protected:
      ICallback() {}
      ~ICallback() {}
    };

  private:
    Timer(const Timer &);
    Timer &operator=(const Timer &);
    Timer();
    ~Timer();
  };

public:
  Server();
  ~Server();

  void setKeepAlive(bool enable = true);
  void setNoDelay(bool enable = true);
  void setSendBufferSize(int size);
  void setReceiveBufferSize(int size);
  void setReuseAddress(bool enable);

  Listener *listen(uint32 addr, uint16 port, Listener::ICallback &callback);
  Establisher *connect(uint32 addr, uint16 port, Establisher::ICallback &callback);
  Establisher *connect(const String &host, uint16 port, Establisher::ICallback &callback);
  Timer *time(int64 interval, Timer::ICallback &callback);
  Client *pair(Client::ICallback &callback, Socket &socket);

  void remove(Client &client);
  void remove(Listener &listener);
  void remove(Establisher &establisher);
  void remove(Timer &timer);

  void run();
  void interrupt();

  void clear();

private:
  Server(const Server &);
  Server &operator=(const Server &);

private:
  class Private;
  Private *_p;
};
