

//#if !defined(_MSC_VER) || defined(_M_AMD64)
//#define USE_PAGE_ALIGNED_ALLOCATION
//#endif

#ifdef USE_PAGE_ALIGNED_ALLOCATION

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <cstring>
#endif
#endif

#include <nstd/Memory.h>
#include <nstd/Debug.h>

#ifdef USE_PAGE_ALIGNED_ALLOCATION

class _Memory
{
public:
    struct PageHeader
    {
      uint64_t checkValue;
      size_t size;
    };

    struct PageFooter 
    {
      uint64_t checkValue;
    };
    
    struct FreePage : public PageHeader
    {
      FreePage* next;
    };

    static size_t pageSize;
    static FreePage* firstFreePage;
    static size_t freePageCount;
#ifdef _WIN32
    static CRITICAL_SECTION criticalSection; // TODO: is it possible to use atomic operations instead?
#else
    static pthread_mutex_t mutex;
#endif
    
    static const uint64_t headerCheckValue = 0x1235543212355432LL;
    static const uint64_t headerCheckValueUsed = 0x1235543212355433LL;
    static const uint64_t footerCheckValue = 0x1235543212355432LL;

    static const size_t maxFreePageCount = 64;

private:
    static _Memory memory;

    _Memory()
    {
#ifdef _WIN32
#ifndef __CYGWIN__
      InitializeCriticalSection(&criticalSection);
      SYSTEM_INFO si;
      GetSystemInfo(&si);
      pageSize = si.dwPageSize;
#endif
#else
      pageSize = sysconf(_SC_PAGESIZE);
#endif
    }
    ~_Memory()
    {
#ifdef _WIN32
      DeleteCriticalSection(&criticalSection);
#endif
    }
};

#ifdef _MSC_VER
#pragma warning(disable: 4073) 
#pragma init_seg(lib)
_Memory _Memory::memory;
#else
_Memory _Memory::memory __attribute__ ((init_priority (101)));
#endif
size_t _Memory::pageSize;
_Memory::FreePage* _Memory::firstFreePage = 0;
size_t _Memory::freePageCount = 0;
#ifdef _WIN32
CRITICAL_SECTION _Memory::criticalSection;
#else
pthread_mutex_t _Memory::mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void_t* Memory::alloc(size_t size)
{
  size_t allocatedSize;
  return alloc(size, allocatedSize);
}

void_t* Memory::alloc(size_t size, size_t& rsize)
{
#ifdef __CYGWIN__
  if(_Memory::pageSize == 0)
  {
      InitializeCriticalSection(&_Memory::criticalSection);
      SYSTEM_INFO si;
      GetSystemInfo(&si);
      _Memory::pageSize = si.dwPageSize;
  }
#else
  ASSERT(_Memory::pageSize > 0);
#endif
  size_t minAllocSize = size + (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
  if(minAllocSize <= _Memory::pageSize)
  {
#ifdef _WIN32
    EnterCriticalSection(&_Memory::criticalSection);
#else
    pthread_mutex_lock(&_Memory::mutex);
#endif
    if(_Memory::firstFreePage)
    {
      _Memory::PageHeader* header = _Memory::firstFreePage;
      _Memory::firstFreePage = _Memory::firstFreePage->next;
      --_Memory::freePageCount;
#ifdef _WIN32
      LeaveCriticalSection(&_Memory::criticalSection);
#else
      pthread_mutex_unlock(&_Memory::mutex);
#endif
      rsize = _Memory::pageSize - (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
      header->checkValue = _Memory::headerCheckValue;
      return (uint8_t*)header + sizeof(_Memory::PageHeader);
    }
    else
    {
#ifdef _WIN32
      LeaveCriticalSection(&_Memory::criticalSection);
#else
      pthread_mutex_unlock(&_Memory::mutex);
#endif
    }
  }
  size_t pageCount = (minAllocSize + _Memory::pageSize - 1) / _Memory::pageSize;
  size_t allocSize = pageCount * _Memory::pageSize;

  _Memory::PageHeader* header;
#ifdef _WIN32
  header = (_Memory::PageHeader*)VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  header = (_Memory::PageHeader*)memalign(_Memory::pageSize, allocSize);
#endif
  if(!header) // out of memory?
  {
    Debug::printf(_T("Memory::alloc: error: Could not allocate %llu pages.\n"), (uint64_t)pageCount); TRAP();
    do // wait and try again...
    {
#ifdef _WIN32
      Sleep(5000);
      header = (_Memory::PageHeader*)VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
      sleep(5);
      header = (_Memory::PageHeader*)memalign(_Memory::pageSize, allocSize);
#endif
    } while(!header);
  }
  _Memory::PageFooter* footer = (_Memory::PageFooter*)((uint8_t*)header + allocSize - sizeof(_Memory::PageFooter));
  header->size = allocSize;
  header->checkValue = _Memory::headerCheckValue;
  footer->checkValue = _Memory::footerCheckValue;
  rsize = allocSize - (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
  return (void_t*)((uint8_t*)header + sizeof(_Memory::PageHeader));
}

size_t Memory::size(void_t* buffer)
{
  if(!buffer) return 0;
  _Memory::PageHeader* header = (_Memory::PageHeader*)((uint8_t*)buffer - sizeof(_Memory::PageHeader));
  if(header->checkValue != _Memory::headerCheckValue)
  {
    if(header->checkValue == _Memory::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::size: error: The passed buffer was freed.\n")); TRAP();
      return 0;
    }
    Debug::print(_T("Memory::size: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return 0;
  }
  return header->size - (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
}
/*
size_t Memory::pageSize()
{
  return _Memory::pageSize;
}
*/
void_t Memory::free(void_t* buffer)
{
  if(!buffer) return;
  _Memory::PageHeader* header = (_Memory::PageHeader*)((uint8_t*)buffer - sizeof(_Memory::PageHeader));
  if(header->checkValue != _Memory::headerCheckValue)
  {
    if(header->checkValue == _Memory::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::free: error: The passed buffer was already freed.\n")); TRAP();
      return;
    }
    Debug::print(_T("Memory::free: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return;
  }
  _Memory::PageFooter* footer = (_Memory::PageFooter*)((uint8_t*)header + header->size - sizeof(_Memory::PageFooter));
  if(footer->checkValue != _Memory::footerCheckValue)
  {
    Debug::print(_T("Memory::free: error: The passed buffer is corrupted.\n")); TRAP(); // buffer overrun?
    footer->checkValue = _Memory::footerCheckValue;
  }

  if(header->size > _Memory::pageSize || _Memory::freePageCount >= _Memory::maxFreePageCount)
  {
#ifdef _WIN32
    VERIFY(VirtualFree(header, 0, MEM_RELEASE));
#else
    ::free(header);
#endif
    return;
  }

  header->checkValue = _Memory::headerCheckValueUsed;
  ((_Memory::FreePage*)header)->next = 0;

#ifdef _WIN32
  EnterCriticalSection(&_Memory::criticalSection);
#else
  pthread_mutex_lock(&_Memory::mutex);
#endif
  ((_Memory::FreePage*)header)->next = _Memory::firstFreePage;
  _Memory::firstFreePage = (_Memory::FreePage*)header;
  ++_Memory::freePageCount;
#ifdef _WIN32
  LeaveCriticalSection(&_Memory::criticalSection);
#else
  pthread_mutex_unlock(&_Memory::mutex);
#endif
}

void_t* operator new(size_t size)
{
  size_t allocatedSize;
  return Memory::alloc(size, allocatedSize);
}

void_t* operator new [](size_t size)
{
  size_t allocatedSize;
  return Memory::alloc(size, allocatedSize);
}

void_t operator delete(void_t* buffer)
{
  Memory::free(buffer);
}

void_t operator delete[](void_t* buffer)
{
  Memory::free(buffer);
}

#else // !USE_PAGE_ALIGNED_ALLOCATION

#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdlib>
#include <malloc.h>
#include <unistd.h>
#include <cstring>
#endif

class _Memory
{
public:
    struct PageHeader
    {
      uint64_t checkValue;
      size_t size;
    };

    struct PageFooter 
    {
      uint64_t checkValue;
    };
    
    struct FreePage : public PageHeader
    {
      FreePage* next;
    };

    static const uint64_t headerCheckValue = 0x1235543212355432LL;
    static const uint64_t headerCheckValueUsed = 0x1235543212355433LL;
    static const uint64_t footerCheckValue = 0x1235543212355432LL;

#ifdef _WIN32
    static HANDLE processHeap;
#endif
private:
    static _Memory memory;

    _Memory()
    {
#ifdef _WIN32
      processHeap = GetProcessHeap();
#endif
    }
};

#ifdef _MSC_VER
#pragma warning(disable: 4073) 
#pragma init_seg(lib)
_Memory _Memory::memory;
#else
_Memory _Memory::memory __attribute__ ((init_priority (101)));
#endif
#ifdef _WIN32
HANDLE _Memory::processHeap = 0;
#endif

void_t* Memory::alloc(size_t minSize, size_t& rsize)
{
#ifdef _WIN32
  ASSERT(_Memory::processHeap);
#endif
  size_t minAllocSize = minSize + (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
  _Memory::PageHeader* header;
#ifdef _WIN32
  header = (_Memory::PageHeader*)HeapAlloc(_Memory::processHeap, 0, minAllocSize);
#else
  header = (_Memory::PageHeader*)malloc(minAllocSize);
#endif
  if(!header) // out of memory?
  {
    Debug::printf(_T("Memory::alloc: error: Could not allocate %llu bytes.\n"), (uint64_t)minAllocSize); TRAP();
    do // wait and try again...
    {
#ifdef _WIN32
      Sleep(5000);
      header = (_Memory::PageHeader*)HeapAlloc(_Memory::processHeap, 0, minAllocSize);
#else
      sleep(5);
      header = (_Memory::PageHeader*)malloc(minAllocSize);
#endif
    } while(!header);
  }
  size_t allocSize;
#ifdef _WIN32
  allocSize = HeapSize(_Memory::processHeap, 0, header);
#else
  allocSize = malloc_usable_size(header);
#endif
  _Memory::PageFooter* footer = (_Memory::PageFooter*)((uint8_t*)header + allocSize - sizeof(_Memory::PageFooter));
  header->size = allocSize;
  header->checkValue = _Memory::headerCheckValue;
  footer->checkValue = _Memory::footerCheckValue;
  rsize = allocSize - (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
  return (void_t*)((uint8_t*)header + sizeof(_Memory::PageHeader));
}

void_t* Memory::alloc(size_t size)
{
  size_t allocatedSize;
  return alloc(size, allocatedSize);
}

size_t Memory::size(void_t* buffer)
{
  if(!buffer) return 0;
  _Memory::PageHeader* header = (_Memory::PageHeader*)((uint8_t*)buffer - sizeof(_Memory::PageHeader));
  if(header->checkValue != _Memory::headerCheckValue)
  {
    if(header->checkValue == _Memory::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::size: error: The passed buffer was freed.\n")); TRAP();
      return 0;
    }
    Debug::print(_T("Memory::size: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return 0;
  }
  return header->size - (sizeof(_Memory::PageHeader) + sizeof(_Memory::PageFooter));
}

void_t Memory::free(void_t* buffer)
{
  if(!buffer) return;
  _Memory::PageHeader* header = (_Memory::PageHeader*)((uint8_t*)buffer - sizeof(_Memory::PageHeader));
  if(header->checkValue != _Memory::headerCheckValue)
  {
    if(header->checkValue == _Memory::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::free: error: The passed buffer was already freed.\n")); TRAP();
      return;
    }
    Debug::print(_T("Memory::free: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return;
  }
  _Memory::PageFooter* footer = (_Memory::PageFooter*)((uint8_t*)header + header->size - sizeof(_Memory::PageFooter));
  if(footer->checkValue != _Memory::footerCheckValue)
  {
    Debug::print(_T("Memory::free: error: The passed buffer is corrupted.\n")); TRAP(); // buffer overrun?
    footer->checkValue = _Memory::footerCheckValue;
  }
#ifdef _WIN32
  VERIFY(HeapFree(_Memory::processHeap, 0, header));
#else
  ::free(header);
#endif
}

void_t* operator new(size_t size)
{
  size_t allocatedSize;
  return Memory::alloc(size, allocatedSize);
}

void_t* operator new [](size_t size)
{
  size_t allocatedSize;
  return Memory::alloc(size, allocatedSize);
}

void_t operator delete(void_t* buffer)
{
  Memory::free(buffer);
}

void_t operator delete[](void_t* buffer)
{
  Memory::free(buffer);
}

#endif // !USE_PAGE_ALIGNED_ALLOCATION

void_t Memory::copy(void_t* dest, const void_t* src, size_t length)
{
#ifdef _WIN32
  CopyMemory(dest, src, length);
#else
  memcpy(dest, src, length);
#endif
}

void_t Memory::move(void_t* dest, const void_t* src, size_t length)
{
#ifdef _WIN32
  MoveMemory(dest, src, length);
#else
  memmove(dest, src, length);
#endif
}

void_t Memory::fill(void_t* buffer, byte_t value, size_t size)
{
#ifdef _WIN32
  FillMemory(buffer, size, value);
#else
  memset(buffer, value, size);
#endif
}

void_t Memory::zero(void_t* buffer, size_t size)
{
#ifdef _WIN32
  ZeroMemory(buffer, size);
#else
  memset(buffer, 0, size);
#endif
}

int_t Memory::compare(const void_t* ptr1, const void_t* ptr2, size_t count)
{
  return memcmp(ptr1, ptr2, count);
}
