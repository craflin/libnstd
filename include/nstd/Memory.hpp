
#pragma once

#include <nstd/Base.hpp>

class Memory
{
public:
  static void* alloc(usize minSize, usize& size);
  static void* alloc(usize size);
  static usize size(void* buffer);
  //static usize pageSize();
  static void free(void* buffer);
  static void copy(void* dest, const void* src, usize count);
  static void move(void* dest, const void* src, usize count);
  static void fill(void* buffer, byte value, usize count);
  static void zero(void* buffer, usize count);
  static int compare(const void* ptr1, const void* ptr2, usize count);

#ifndef NDEBUG
  static void dump();
#endif

private:
  class Private;

  friend void* operator new(usize size);
  friend void* operator new [](usize size);
  friend void operator delete(void* buffer);
  friend void operator delete[](void* buffer);
};
