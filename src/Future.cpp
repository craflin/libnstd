
#include <nstd/Future.hpp>
#include <nstd/Atomic.hpp>
#include <nstd/Signal.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Time.hpp>
#include <nstd/Mutex.hpp>
#include <nstd/PoolList.hpp>
#include <nstd/System.hpp>

class Future<void>::Private
{
public:
  template <typename T>
  class LockFreeQueue
  {
  public:
    inline LockFreeQueue(usize capacity);
    inline ~LockFreeQueue();
    inline usize capacity() const { return _capacity; }
    inline usize size() const;
    inline bool push(const T &data);
    inline bool pop(T &result);

  private:
    struct Node
    {
      T data;
      volatile usize tail;
      volatile usize head;
    };

  private:
    usize _capacity;
    usize _capacityMask;
    Node *_queue;
    char _cacheLinePad1[64];
    volatile usize _tail;
    char _cacheLinePad2[64];
    volatile usize _head;
    char _cacheLinePad3[64];
  };

  class FastSignal
  {
  public:
    inline FastSignal() : _state(0) {}
    inline void set();
    inline void reset();
    inline bool wait();

  private:
    Signal _signal;
    volatile usize _state;
  };

  class ThreadPool
  {
  public:
    ThreadPool(usize minThreads = 0, usize maxThreads = System::getProcessorCount(), usize queueSize = 0x100)
        : _minThreads(minThreads), _maxThreads(maxThreads), _queue(queueSize), _pushedJobs(0), _processedJobs(0), _threadCount(0)
    {
      if (_maxThreads < 3)
        _maxThreads = 3;
    }

    ~ThreadPool()
    {
      Job job = {0, 0};
      for (PoolList<ThreadContext>::Iterator i = _threads.begin(), end = _threads.end(); i != end; ++i)
      {
        while (!_queue.push(job))
        {
          _dequeuedSignal.reset();
          if (_queue.push(job))
            break;
          _dequeuedSignal.wait();
        }
        _enqueuedSignal.set();
      }
      for (PoolList<ThreadContext>::Iterator i = _threads.begin(), end = _threads.end(); i != end; ++i)
        i->_thread.join();
    }

    void run(void (*proc)(void *), void *args)
    {
      Job job = {proc, args};
      while (!_queue.push(job))
      {
        _dequeuedSignal.reset();
        if (_queue.push(job))
          break;
        _dequeuedSignal.wait();
      }
      _enqueuedSignal.set();

      // adjust worker thread count
      usize pushedJobs = Atomic::increment(_pushedJobs);
      ssize busyThreads = (ssize)(pushedJobs - _processedJobs);
      usize threadCount = _threadCount;
      ssize idleThreads = (ssize)threadCount - busyThreads;
      if (idleThreads == 1)
        _idleResetTime = (uint32)(Time::ticks() >> 10);
      else
      {
        if (idleThreads <= 0)
        { // start new worker thread
          _idleResetTime = (uint32)(Time::ticks() >> 10);
          if (threadCount < _maxThreads)
          {
            ThreadContext *context = 0;
            {
              Mutex::Guard guard(_mutex);
              for (PoolList<ThreadContext>::Iterator i = _threads.begin(), end = _threads.end(); i != end;)
              {
                if (i->_terminated)
                  i = _threads.remove(i);
                else
                  ++i;
              }
              if (_threadCount < _maxThreads)
              {
                ++_threadCount;
                context = &_threads.append();
              }
            }
            if (context)
            {
              context->_pool = this;
              if (!context->_thread.start(*context, &ThreadContext::proc))
                context->_terminated = true;
            }
          }
        }
        else if (idleThreads > 1 && threadCount > _minThreads && (uint32)(Time::ticks() >> 10) - _idleResetTime > 1)
        { // terminate a worker thread
          Mutex::Guard guard(_mutex);
          if (_threadCount > _minThreads)
          {
            Job job = {0, 0};
            if (_queue.push(job))
              --_threadCount;
          }
          for (PoolList<ThreadContext>::Iterator i = _threads.begin(), end = _threads.end(); i != end;)
          {
            if (i->_terminated)
              i = _threads.remove(i);
            else
              ++i;
          }
        }
      }
    }

  private:
    struct Job
    {
      void (*proc)(void *);
      void *args;
    };

    struct ThreadContext
    {
      Thread _thread;
      volatile bool _terminated;
      ThreadPool *_pool;
      ThreadContext() : _terminated(false) {}
      uint proc()
      {
        Job job;
        LockFreeQueue<Job> &queue = _pool->_queue;
        FastSignal &enqueuedSignal = _pool->_enqueuedSignal;
        FastSignal &dequeuedSignal = _pool->_dequeuedSignal;
        for (;;)
        {
          while (!queue.pop(job))
          {
            enqueuedSignal.reset();
            if (queue.pop(job))
              break;
            enqueuedSignal.wait();
          }
          dequeuedSignal.set();
          if (job.proc)
          {
            job.proc(job.args);
            Atomic::increment(_pool->_processedJobs);
          }
          else
            break;
        }
        _terminated = true;
        return 0;
      }
    };

  private:
    usize _minThreads;
    usize _maxThreads;
    LockFreeQueue<Job> _queue;
    FastSignal _dequeuedSignal;
    FastSignal _enqueuedSignal;
    volatile usize _pushedJobs;
    volatile usize _processedJobs;
    volatile usize _threadCount;
    volatile uint32 _idleResetTime;
    Mutex _mutex;
    PoolList<ThreadContext> _threads;
  };

  static ThreadPool *volatile _threadPool;
  static volatile int _threadPoolLock;

  class Framework
  {
    ~Framework();
    static Framework framework;
  };
};

Future<void>::Private::ThreadPool *volatile Future<void>::Private::_threadPool = 0;
volatile int Future<void>::Private::_threadPoolLock = 0;
Future<void>::Private::Framework Future<void>::Private::Framework::framework;

Future<void>::Private::Framework::~Framework()
{
  if (_threadPool)
    delete _threadPool;
}

void Future<void>::set()
{
  Atomic::swap(_state, _aborting ? abortedState : finishedState);
  _sig.set();
}

void Future<void>::startProc(void (*proc)(void *), void *args)
{
  Private::ThreadPool *threadPool = Private::_threadPool;
  if (!threadPool)
  {
    while (Atomic::testAndSet(Private::_threadPoolLock) != 0)
      ;
    if (!(threadPool = Private::_threadPool))
    {
      threadPool = new Private::ThreadPool;
      Atomic::swap(Private::_threadPool, threadPool);
    }
    Private::_threadPoolLock = 0;
  }

  join();
  _joinable = true;
  _aborting = false;
  threadPool->run(proc, args);
}

template <typename T>
inline Future<void>::Private::LockFreeQueue<T>::LockFreeQueue(usize capacity)
{
  _capacityMask = capacity - 1;
  _capacityMask |= _capacityMask >> 1;
  _capacityMask |= _capacityMask >> 2;
  _capacityMask |= _capacityMask >> 4;
  _capacityMask |= _capacityMask >> 8;
  _capacityMask |= _capacityMask >> 16;
  _capacity = _capacityMask + 1;

  _queue = (Node *)Memory::alloc(sizeof(Node) * _capacity);
  for (usize i = 0; i < _capacity; ++i)
  {
    _queue[i].tail = i;
    _queue[i].head = -1;
  }

  _tail = 0;
  _head = 0;
}

template <typename T>
inline Future<void>::Private::LockFreeQueue<T>::~LockFreeQueue()
{
  for (usize i = _head; i != _tail; ++i)
    (&_queue[i % _capacity].data)->~T();

  Memory::free(_queue);
}

template <typename T>
inline usize Future<void>::Private::LockFreeQueue<T>::size() const
{
  usize head = _head;
  Atomic::memoryBarrier();
  return _tail - head;
}

template <typename T>
inline bool Future<void>::Private::LockFreeQueue<T>::push(const T &data)
{
  Node *node;
  usize next, tail = _tail;
  for (;; tail = next)
  {
    node = &_queue[tail & _capacityMask];
    if (node->tail != tail)
      return false;
    if ((next = Atomic::compareAndSwap(_tail, tail, tail + 1)) == tail)
      break;
  }
  new (&node->data) T(data);
  Atomic::swap(node->head, tail);
  return true;
}

template <typename T>
inline bool Future<void>::Private::LockFreeQueue<T>::pop(T &result)
{
  Node *node;
  usize next, head = _head;
  for (;; head = next)
  {
    node = &_queue[head & _capacityMask];
    if (node->head != head)
      return false;
    if ((next = Atomic::compareAndSwap(_head, head, head + 1)) == head)
      break;
  }
  result = node->data;
  (&node->data)->~T();
  Atomic::swap(node->tail, head + _capacity);
  return true;
}

void Future<void>::Private::FastSignal::set()
{
  if (Atomic::testAndSet(_state) == 0)
    _signal.set();
}

void Future<void>::Private::FastSignal::reset()
{
  if (Atomic::swap(_state, 0) == 1)
    _signal.reset();
}

bool Future<void>::Private::FastSignal::wait()
{
  if (Atomic::load(_state))
    return true;
  return _signal.wait();
}
