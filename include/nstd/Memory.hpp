
#pragma once

#include <nstd/Base.hpp>

class Memory
{
public:
  static void copy(void* dest, const void* src, usize count);
  static void move(void* dest, const void* src, usize count);
  static void fill(void* buffer, byte value, usize count);
  static void zero(void* buffer, usize count);
  static int compare(const void* ptr1, const void* ptr2, usize count);
};
