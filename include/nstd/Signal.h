
#pragma once

#include <nstd/Base.h>

class Signal
{
public:
  Signal(bool set = false);
  ~Signal();

  void_t set();
  void_t reset();
  bool_t wait();

private:
  void* handle;

  Signal(const Signal&);
  Signal& operator=(const Signal&);
};
