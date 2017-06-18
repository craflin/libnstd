
#include <nstd/Future.h>
#include <nstd/Atomic.h>

void Future<void>::set()
{
  Atomic::swap(state, aborting ? abortedState : finishedState);
  sig.set();
}

void Future<void>::startProc(uint (*proc)(void*), void* args)
{
  join();
  aborting = false;
  state = runningState;
  sig.reset();
  //if(!thread.start(proc, args))
    proc(args);
}
