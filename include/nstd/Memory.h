
#pragma once

#include <nstd/Base.h>

class Memory
{
public:
  static void_t* alloc(size_t minSize, size_t& size);
  static void_t* alloc(size_t size);
  static size_t size(void_t* buffer);
  //static size_t pageSize();
  static void_t free(void_t* buffer);
  static void_t copy(void_t* dest, const void_t* src, size_t count);
  static void_t move(void_t* dest, const void_t* src, size_t count);
  static void_t fill(void_t* buffer, byte_t value, size_t count);
  static void_t zero(void_t* buffer, size_t count);
  static int_t compare(const void_t* ptr1, const void_t* ptr2, size_t count);

private:
  class Private;

  friend void_t* operator new(size_t size);
  friend void_t* operator new [](size_t size);
  friend void_t operator delete(void_t* buffer);
  friend void_t operator delete[](void_t* buffer);
};
