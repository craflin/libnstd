
#include <nstd/Future.h>
#include <nstd/Atomic.h>
#include <nstd/Signal.h>
#include <nstd/Thread.h>
#include <nstd/Time.h>
#include <nstd/Mutex.h>
#include <nstd/PoolList.h>
#include <nstd/System.h>

class Future<void>::Private
{
public:
  template <typename T> class LockFreeQueue
  {
  public:
    inline LockFreeQueue(usize capacity);
    inline ~LockFreeQueue();
    inline usize capacity() const {return _capacity;}
    inline usize size() const;
    inline bool push(const T& data);
    inline bool pop(T& result);

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
    Node* _queue;
    char cacheLinePad1[64];
    volatile usize _tail;
    char cacheLinePad2[64];
    volatile usize _head;
    char cacheLinePad3[64];
  };

  class FastSignal
  {
  public:
    inline FastSignal() : state(0) {}
    inline void set();
    inline void reset();
    inline bool wait();
  private:
    Signal signal;
    volatile usize state;
  };

  class ThreadPool
  {
  public:
    ThreadPool(usize minThreads = 0, usize maxThreads = System::getProcessorCount(), usize queueSize = 0x100)
      : minThreads(minThreads)
      , maxThreads(maxThreads)
      , queue(queueSize) 
      , pushedJobs(0)
      , processedJobs(0)
      , threadCount(0) {}
    ~ThreadPool()
    {
      if(this->maxThreads < 3)
        this->maxThreads = 3;

      Job job = {0, 0};
      for(PoolList<ThreadContext>::Iterator i = threads.begin(), end = threads.end(); i != end; ++i)
      {
        if(i->terminated)
          continue;
        while(!queue.push(job))
        {
          dequeuedSignal.reset();
          if(queue.push(job))
            break;
          dequeuedSignal.wait();
        }
        enqueuedSignal.set();
      }
    }

    void run(void (*proc)(void*), void* args)
    {
      Job job = {proc, args};
      while(!queue.push(job))
      {
        dequeuedSignal.reset();
        if(queue.push(job))
          break;
        dequeuedSignal.wait();
      }
      enqueuedSignal.set();

      // adjust worker thread count
      usize pushedJobs = Atomic::increment(this->pushedJobs);
      ssize busyThreads = (ssize)(pushedJobs - processedJobs);
      usize threadCount = this->threadCount;
      ssize idleThreads = (ssize)threadCount - busyThreads;
      if(idleThreads == 1)
        idleResetTime = (uint32)(Time::time() >> 10);
      else
      {
        if(idleThreads <= 0)
        { // start new worker thread
          idleResetTime = (uint32)(Time::time() >> 10);
          if(threadCount < maxThreads)
          {
            ThreadContext* context = 0;
            mutex.lock();
            for(PoolList<ThreadContext>::Iterator i = threads.begin(), end = threads.end(); i != end;)
            {
              if(i->terminated)
                i = threads.remove(i);
              else
                ++i;
            }
            if(this->threadCount < maxThreads)
            {
              ++this->threadCount;
              context = &threads.append();
            }
            mutex.unlock();
            if(context)
            {
              context->pool = this;
              if(!context->thread.start(*context, &ThreadContext::proc))
                context->terminated = true;
            }
          }
        }
        else if(idleThreads > 1 && threadCount > minThreads && (uint32)(Time::time() >> 10) - idleResetTime > 1)
        { // terminate a worker thread
          mutex.lock();
          if(this->threadCount > minThreads)
          {
            Job job = {0, 0};
            if(queue.push(job))
              --this->threadCount;
          }
          for(PoolList<ThreadContext>::Iterator i = threads.begin(), end = threads.end(); i != end;)
          {
            if(i->terminated)
              i = threads.remove(i);
            else
              ++i;
          }
          mutex.unlock();
        }
      }
    }

  private:
    struct Job
    {
      void (*proc)(void*);
      void* args;
    };

    struct ThreadContext
    {
      Thread thread;
      volatile bool terminated;
      ThreadPool* pool;
      ThreadContext() : terminated(false) {}
      uint proc()
      {
        Job job;
        LockFreeQueue<Job>& queue = pool->queue;
        FastSignal& enqueuedSignal = pool->enqueuedSignal; 
        FastSignal& dequeuedSignal = pool->dequeuedSignal; 
        for(;;)
        {
          while(!queue.pop(job))
          {
            enqueuedSignal.reset();
            if(queue.pop(job))
              break;
            enqueuedSignal.wait();
          }
          dequeuedSignal.set();
          if(job.proc)
          {
            job.proc(job.args);
            Atomic::increment(pool->processedJobs);
          }
          else
            break;
        }
        terminated = true;
        return 0;
      }
    };

  private:
    usize minThreads;
    usize maxThreads;
    LockFreeQueue<Job> queue;
    FastSignal dequeuedSignal;
    FastSignal enqueuedSignal;
    volatile usize pushedJobs;
    volatile usize processedJobs;
    volatile usize threadCount;
    volatile uint32 idleResetTime;
    Mutex mutex;
    PoolList<ThreadContext> threads;
  };

  static ThreadPool* volatile threadPool;
  static volatile int threadPoolLock;

  class Framework
  {
    ~Framework();
    static Framework framework;
  };

};

Future<void>::Private::ThreadPool* volatile Future<void>::Private::threadPool = 0;
volatile int Future<void>::Private::threadPoolLock = 0;
Future<void>::Private::Framework Future<void>::Private::Framework::framework;

Future<void>::Private::Framework::~Framework()
{
  if(threadPool)
    delete threadPool;
}

void Future<void>::set()
{
  Atomic::swap(state, aborting ? abortedState : finishedState);
  sig.set();
}

void Future<void>::startProc(void (*proc)(void*), void* args)
{
  Private::ThreadPool*  threadPool = Private::threadPool;
  if(!threadPool)
  {
    while(Atomic::testAndSet(Private::threadPoolLock) != 0);
    if(!(threadPool = Private::threadPool))
    {
      threadPool = new Private::ThreadPool;
      Atomic::swap(Private::threadPool, threadPool);
    }
    Private::threadPoolLock = 0;
  }

  join();
  aborting = false;
  state = runningState;
  sig.reset();
  threadPool->run(proc, args);
}

template<typename T> inline Future<void>::Private::LockFreeQueue<T>::LockFreeQueue(usize capacity)
{
  _capacityMask = capacity - 1;
  _capacityMask |= _capacityMask >> 1;
  _capacityMask |= _capacityMask >> 2;
  _capacityMask |= _capacityMask >> 4;
  _capacityMask |= _capacityMask >> 8;
  _capacityMask |= _capacityMask >> 16;
  _capacity = _capacityMask + 1;

  _queue = (Node*)Memory::alloc(sizeof(Node) * _capacity);
  for(usize i = 0; i < _capacity; ++i)
  {
    _queue[i].tail = i;
    _queue[i].head = -1;
  }

  _tail = 0;
  _head = 0;
}

template<typename T> inline Future<void>::Private::LockFreeQueue<T>::~LockFreeQueue()
{
  for(usize i = _head; i != _tail; ++i)
    (&_queue[i % _capacity].data)->~T();

  Memory::free(_queue);
}

template<typename T> inline usize Future<void>::Private::LockFreeQueue<T>::size() const
{
  usize head = _head;
  Atomic::memoryBarrier();
  return _tail - head;
}
  
template<typename T> inline bool Future<void>::Private::LockFreeQueue<T>::push(const T& data)
{
  Node* node;
  usize next, tail = _tail;
  for(;; tail = next)
  {
    node = &_queue[tail & _capacityMask];
    if(node->tail != tail)
      return false;
    if((next = Atomic::compareAndSwap(_tail, tail, tail + 1)) == tail)
      break;
  }
  new (&node->data)T(data);
  Atomic::swap(node->head, tail);
  return true;
}

template<typename T> inline bool Future<void>::Private::LockFreeQueue<T>::pop(T& result)
{
  Node* node;
  usize next, head = _head;
  for(;; head = next)
  {
    node = &_queue[head & _capacityMask];
    if(node->head != head)
      return false;
    if((next = Atomic::compareAndSwap(_head, head, head + 1)) == head)
      break;
  }
  result = node->data;
  (&node->data)->~T();
  Atomic::swap(node->tail, head + _capacity);
  return true;
}

void Future<void>::Private::FastSignal::set()
{
  if(Atomic::testAndSet(state) == 0)
    signal.set();
}

void Future<void>::Private::FastSignal::reset()
{
  if(Atomic::swap(state, 0) == 1)
    signal.reset();
}

bool Future<void>::Private::FastSignal::wait()
{
  if(Atomic::load(state))
    return true;
  return signal.wait();
}
