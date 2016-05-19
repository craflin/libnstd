

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

class Memory::Private
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
    static Private memory;

    Private()
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
    ~Private()
    {
#ifdef _WIN32
      DeleteCriticalSection(&criticalSection);
#endif
    }
};

#ifdef _MSC_VER
#pragma warning(disable: 4073) 
#pragma init_seg(lib)
Memory::Private Memory::Private::memory;
#else
Memory::Private Memory::Private::memory __attribute__ ((init_priority (101)));
#endif
size_t Memory::Private::pageSize;
Memory::Private::FreePage* Memory::Private::firstFreePage = 0;
size_t Memory::Private::freePageCount = 0;
#ifdef _WIN32
CRITICAL_SECTION Memory::Private::criticalSection;
#else
pthread_mutex_t Memory::Private::mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void_t* Memory::alloc(size_t size)
{
  size_t allocatedSize;
  return alloc(size, allocatedSize);
}

void_t* Memory::alloc(size_t size, size_t& rsize)
{
#ifdef __CYGWIN__
  if(Memory::Private::pageSize == 0)
  {
      InitializeCriticalSection(&Memory::Private::criticalSection);
      SYSTEM_INFO si;
      GetSystemInfo(&si);
      Memory::Private::pageSize = si.dwPageSize;
  }
#else
  ASSERT(Memory::Private::pageSize > 0);
#endif
  size_t minAllocSize = size + (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
  if(minAllocSize <= Memory::Private::pageSize)
  {
#ifdef _WIN32
    EnterCriticalSection(&Memory::Private::criticalSection);
#else
    pthread_mutex_lock(&Memory::Private::mutex);
#endif
    if(Memory::Private::firstFreePage)
    {
      Memory::Private::PageHeader* header = Memory::Private::firstFreePage;
      Memory::Private::firstFreePage = Memory::Private::firstFreePage->next;
      --Memory::Private::freePageCount;
#ifdef _WIN32
      LeaveCriticalSection(&Memory::Private::criticalSection);
#else
      pthread_mutex_unlock(&Memory::Private::mutex);
#endif
      rsize = Memory::Private::pageSize - (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
      header->checkValue = Memory::Private::headerCheckValue;
      return (uint8_t*)header + sizeof(Memory::Private::PageHeader);
    }
    else
    {
#ifdef _WIN32
      LeaveCriticalSection(&Memory::Private::criticalSection);
#else
      pthread_mutex_unlock(&Memory::Private::mutex);
#endif
    }
  }
  size_t pageCount = (minAllocSize + Memory::Private::pageSize - 1) / Memory::Private::pageSize;
  size_t allocSize = pageCount * Memory::Private::pageSize;

  Memory::Private::PageHeader* header;
#ifdef _WIN32
  header = (Memory::Private::PageHeader*)VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  header = (Memory::Private::PageHeader*)memalign(Memory::Private::pageSize, allocSize);
#endif
  if(!header) // out of memory?
  {
    Debug::printf(_T("Memory::alloc: error: Could not allocate %llu pages.\n"), (uint64_t)pageCount); TRAP();
    do // wait and try again...
    {
#ifdef _WIN32
      Sleep(5000);
      header = (Memory::Private::PageHeader*)VirtualAlloc(NULL, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
      sleep(5);
      header = (Memory::Private::PageHeader*)memalign(Memory::Private::pageSize, allocSize);
#endif
    } while(!header);
  }
  Memory::Private::PageFooter* footer = (Memory::Private::PageFooter*)((uint8_t*)header + allocSize - sizeof(Memory::Private::PageFooter));
  header->size = allocSize;
  header->checkValue = Memory::Private::headerCheckValue;
  footer->checkValue = Memory::Private::footerCheckValue;
  rsize = allocSize - (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
  return (void_t*)((uint8_t*)header + sizeof(Memory::Private::PageHeader));
}

size_t Memory::size(void_t* buffer)
{
  if(!buffer) return 0;
  Memory::Private::PageHeader* header = (Memory::Private::PageHeader*)((uint8_t*)buffer - sizeof(Memory::Private::PageHeader));
  if(header->checkValue != Memory::Private::headerCheckValue)
  {
    if(header->checkValue == Memory::Private::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::size: error: The passed buffer was freed.\n")); TRAP();
      return 0;
    }
    Debug::print(_T("Memory::size: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return 0;
  }
  return header->size - (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
}
/*
size_t Memory::pageSize()
{
  return Memory::Private::pageSize;
}
*/
void_t Memory::free(void_t* buffer)
{
  if(!buffer) return;
  Memory::Private::PageHeader* header = (Memory::Private::PageHeader*)((uint8_t*)buffer - sizeof(Memory::Private::PageHeader));
  if(header->checkValue != Memory::Private::headerCheckValue)
  {
    if(header->checkValue == Memory::Private::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::free: error: The passed buffer was already freed.\n")); TRAP();
      return;
    }
    Debug::print(_T("Memory::free: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return;
  }
  Memory::Private::PageFooter* footer = (Memory::Private::PageFooter*)((uint8_t*)header + header->size - sizeof(Memory::Private::PageFooter));
  if(footer->checkValue != Memory::Private::footerCheckValue)
  {
    Debug::print(_T("Memory::free: error: The passed buffer is corrupted.\n")); TRAP(); // buffer overrun?
    footer->checkValue = Memory::Private::footerCheckValue;
  }

  if(header->size > Memory::Private::pageSize || Memory::Private::freePageCount >= Memory::Private::maxFreePageCount)
  {
#ifdef _WIN32
    VERIFY(VirtualFree(header, 0, MEM_RELEASE));
#else
    ::free(header);
#endif
    return;
  }

  header->checkValue = Memory::Private::headerCheckValueUsed;
  ((Memory::Private::FreePage*)header)->next = 0;

#ifdef _WIN32
  EnterCriticalSection(&Memory::Private::criticalSection);
#else
  pthread_mutex_lock(&Memory::Private::mutex);
#endif
  ((Memory::Private::FreePage*)header)->next = Memory::Private::firstFreePage;
  Memory::Private::firstFreePage = (Memory::Private::FreePage*)header;
  ++Memory::Private::freePageCount;
#ifdef _WIN32
  LeaveCriticalSection(&Memory::Private::criticalSection);
#else
  pthread_mutex_unlock(&Memory::Private::mutex);
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
#ifdef _MSC_VER
#include <intrin.h>
#endif
#else
#include <cstdlib>
#include <malloc.h>
#include <unistd.h>
#include <cstring>
#endif

#ifndef NDEBUG
#include <nstd/HashMap.h>
#include <nstd/String.h>
#include <nstd/MultiMap.h>
#endif

class Memory::Private
{
public:
    struct PageHeader
    {
      size_t size;
      void* returnAddr;
#ifndef NDEBUG
      PageHeader* next;
      PageHeader** previous;
#endif
      uint64_t checkValue;
    };

    struct PageFooter 
    {
      uint64_t checkValue;
    };

    static const uint64_t headerCheckValue = 0x1235543212355432LL;
    static const uint64_t headerCheckValueUsed = 0x1235543212355433LL;
    static const uint64_t footerCheckValue = 0x1235543212355432LL;

#ifdef _WIN32
    static HANDLE processHeap;
#endif
#ifndef NDEBUG
#ifdef _WIN32
    static CRITICAL_SECTION criticalSection; // TODO: is it possible to use atomic operations instead?
#else
    // todo
#endif
    static PageHeader* first;
#endif

public:
  static inline void_t* alloc(size_t minSize, size_t& rsize, void* returnAddr);
  static inline void_t free(void_t* buffer);

private:
    static Private memory;

    Private()
    {
#ifdef _WIN32
      processHeap = GetProcessHeap();
#endif
#ifndef NDEBUG
#ifdef _WIN32
      InitializeCriticalSection(&criticalSection);
#else
      // todo
#endif
#endif
    }

#ifndef NDEBUG
    ~Private()
    {
      for(PageHeader* i = first; i; i = i->next)
      {
        const tchar_t* file;
        int_t line;
        if(!Debug::getSymbol(i->returnAddr, file, line))
          Debug::printf(_T("%p: Found memory leak.\n"), i->returnAddr);
        else
#ifdef _MSC_VER
          Debug::printf(_T("%s(%d): Found memory leak.\n"), file, line);
#else
          Debug::printf(_T("%s:%d: Found memory leak.\n"), file, line);
#endif
      }
      if(first)
        TRAP();
#ifdef _WIN32
      DeleteCriticalSection(&criticalSection);
#endif
    }
#endif
};

#ifdef _MSC_VER
#pragma warning(disable: 4073) 
#pragma init_seg(lib)
Memory::Private Memory::Private::memory;
#else
Memory::Private Memory::Private::memory __attribute__ ((init_priority (101)));
#endif
#ifdef _WIN32
HANDLE Memory::Private::processHeap = 0;
#endif
#ifndef NDEBUG
#ifdef _WIN32
CRITICAL_SECTION Memory::Private::criticalSection;
#endif
Memory::Private::PageHeader* Memory::Private::first = 0;
#endif

void_t* Memory::Private::alloc(size_t minSize, size_t& rsize, void* returnAddr)
{
#ifdef _WIN32
  ASSERT(Memory::Private::processHeap);
#endif
  size_t minAllocSize = minSize + (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
  Memory::Private::PageHeader* header;
#ifdef _WIN32
  header = (Memory::Private::PageHeader*)HeapAlloc(Memory::Private::processHeap, 0, minAllocSize);
#else
  header = (Memory::Private::PageHeader*)malloc(minAllocSize);
#endif
  if(!header) // out of memory?
  {
    Debug::printf(_T("Memory::alloc: error: Could not allocate %llu bytes.\n"), (uint64_t)minAllocSize); TRAP();
    do // wait and try again...
    {
#ifdef _WIN32
      Sleep(5000);
      header = (Memory::Private::PageHeader*)HeapAlloc(Memory::Private::processHeap, 0, minAllocSize);
#else
      sleep(5);
      header = (Memory::Private::PageHeader*)malloc(minAllocSize);
#endif
    } while(!header);
  }
  size_t allocSize;
#ifdef _WIN32
  allocSize = HeapSize(Memory::Private::processHeap, 0, header);
#else
  allocSize = malloc_usable_size(header);
#endif
  Memory::Private::PageFooter* footer = (Memory::Private::PageFooter*)((uint8_t*)header + allocSize - sizeof(Memory::Private::PageFooter));
  header->size = allocSize;
#ifndef NDEBUG
  header->returnAddr = returnAddr;
#endif
  header->checkValue = Memory::Private::headerCheckValue;
  footer->checkValue = Memory::Private::footerCheckValue;
  rsize = allocSize - (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
#ifndef NDEBUG
  EnterCriticalSection(&Private::criticalSection);
  if((header->next = Private::first))
    header->next->previous = &header->next;
  header->previous = &Private::first;
  Private::first = header;
  LeaveCriticalSection(&Private::criticalSection);
#endif
  return (void_t*)((uint8_t*)header + sizeof(Memory::Private::PageHeader));
}

void_t Memory::Private::free(void_t* buffer)
{
  if(!buffer) return;
  Memory::Private::PageHeader* header = (Memory::Private::PageHeader*)((uint8_t*)buffer - sizeof(Memory::Private::PageHeader));
  if(header->checkValue != Memory::Private::headerCheckValue)
  {
    if(header->checkValue == Memory::Private::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::free: error: The passed buffer was already freed.\n")); TRAP();
      return;
    }
    Debug::print(_T("Memory::free: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return;
  }
  Memory::Private::PageFooter* footer = (Memory::Private::PageFooter*)((uint8_t*)header + header->size - sizeof(Memory::Private::PageFooter));
  if(footer->checkValue != Memory::Private::footerCheckValue)
  {
    Debug::print(_T("Memory::free: error: The passed buffer is corrupted.\n")); TRAP(); // buffer overrun?
    footer->checkValue = Memory::Private::footerCheckValue;
  }
#ifndef NDEBUG
  EnterCriticalSection(&Private::criticalSection);
  if((*(header->previous) = header->next))
    header->next->previous = header->previous;
  LeaveCriticalSection(&Private::criticalSection);
#endif
#ifdef _WIN32
  VERIFY(HeapFree(Memory::Private::processHeap, 0, header));
#else
  ::free(header);
#endif
}

#ifndef NDEBUG
void_t Memory::dump()
{
  HashMap<String, size_t> allocatedMemory;
  EnterCriticalSection(&Private::criticalSection);
  for(Private::PageHeader* i = Private::first; i; i = i->next)
  {
    const tchar_t* file;
    int_t line;
    bool success = Debug::getSymbol(i->returnAddr, file, line);
    String key;
    if(success)
#ifdef _WIN32
      key.printf("%s(%d)", file, line);
#else
      key.printf("%s:%d", file, line);
#endif
    HashMap<String, size_t>::Iterator it = allocatedMemory.find(key);
    if(it == allocatedMemory.end())
      allocatedMemory.append(key, i->size);
    else
      *it += i->size;
  }
  LeaveCriticalSection(&Private::criticalSection);
  MultiMap<size_t, String> sortedAllocatedMemory;
  for(HashMap<String, size_t>::Iterator i = allocatedMemory.begin(), end = allocatedMemory.end(); i != end; ++i)
    sortedAllocatedMemory.insert(*i, i.key());
  for(MultiMap<size_t, String>::Iterator i = sortedAllocatedMemory.begin(), end = sortedAllocatedMemory.end(); i != end; ++i)
    Debug::printf("%s: %llu bytes\n", (const tchar_t*)*i, (uint64_t)i.key());
}
#endif

void_t* Memory::alloc(size_t minSize, size_t& rsize)
{
#ifndef NDEBUG
#ifdef _MSC_VER
  void* returnAddr = _ReturnAddress();
#else
  void* returnAddr = 0; // todo
#endif
#else
  void* returnAddr = 0;
#endif
  return Private::alloc(minSize, rsize, returnAddr);
}

void_t* Memory::alloc(size_t size)
{
#ifndef NDEBUG
#ifdef _MSC_VER
  void* returnAddr = _ReturnAddress();
#else
  void* returnAddr = 0; // todo
#endif
#else
  void* returnAddr = 0;
#endif
  size_t allocatedSize;
  return Private::alloc(size, allocatedSize, returnAddr);
}

size_t Memory::size(void_t* buffer)
{
  if(!buffer) return 0;
  Memory::Private::PageHeader* header = (Memory::Private::PageHeader*)((uint8_t*)buffer - sizeof(Memory::Private::PageHeader));
  if(header->checkValue != Memory::Private::headerCheckValue)
  {
    if(header->checkValue == Memory::Private::headerCheckValueUsed)
    {
      Debug::print(_T("Memory::size: error: The passed buffer was freed.\n")); TRAP();
      return 0;
    }
    Debug::print(_T("Memory::size: error: The passed buffer is invalid or corrupted.\n")); TRAP();
    return 0;
  }
  return header->size - (sizeof(Memory::Private::PageHeader) + sizeof(Memory::Private::PageFooter));
}

void_t Memory::free(void_t* buffer)
{
  Private::free(buffer);
}

void_t* operator new(size_t size)
{
#ifndef NDEBUG
#ifdef _MSC_VER
  void* returnAddr = _ReturnAddress();
#else
  void* returnAddr = 0; // todo
#endif
#else
  void* returnAddr = 0;
#endif
  size_t allocatedSize;
  return Memory::Private::alloc(size, allocatedSize, returnAddr);
}

void_t* operator new [](size_t size)
{
#ifndef NDEBUG
#ifdef _MSC_VER
  void* returnAddr = _ReturnAddress();
#else
  void* returnAddr = 0; // todo
#endif
#else
  void* returnAddr = 0;
#endif
  size_t allocatedSize;
  return Memory::Private::alloc(size, allocatedSize, returnAddr);
}

void_t operator delete(void_t* buffer)
{
  Memory::Private::free(buffer);
}

void_t operator delete[](void_t* buffer)
{
  Memory::Private::free(buffer);
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
