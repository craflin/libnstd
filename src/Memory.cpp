
#include <nstd/Memory.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <cstring>

void Memory::copy(void* dest, const void* src, usize length)
{
#ifdef _WIN32
  CopyMemory(dest, src, length);
#else
  memcpy(dest, src, length);
#endif
}

void Memory::move(void* dest, const void* src, usize length)
{
#ifdef _WIN32
  MoveMemory(dest, src, length);
#else
  memmove(dest, src, length);
#endif
}

void Memory::fill(void* buffer, byte value, usize size)
{
#ifdef _WIN32
  FillMemory(buffer, size, value);
#else
  memset(buffer, value, size);
#endif
}

void Memory::zero(void* buffer, usize size)
{
#ifdef _WIN32
  ZeroMemory(buffer, size);
#else
  memset(buffer, 0, size);
#endif
}

int Memory::compare(const void* ptr1, const void* ptr2, usize count)
{
  return memcmp(ptr1, ptr2, count);
}
