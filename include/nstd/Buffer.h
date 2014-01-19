
#pragma once

#include <nstd/Memory.h>

class Buffer
{
public:
  Buffer() : buffer(0), bufferStart((byte_t*)&_capacity), bufferEnd(bufferStart), _capacity(0) {}

  explicit Buffer(size_t capacity) 
  {
    bufferStart = bufferEnd = buffer = (byte_t*)Memory::alloc(capacity + 1, _capacity); --_capacity;
    *bufferEnd = 0;
  }

  Buffer(const Buffer& other)
  {
    size_t size = other.bufferEnd - other.bufferStart;
    bufferStart = buffer = (byte_t*)Memory::alloc(size + 1, _capacity); --_capacity;
    Memory::copy(buffer, other.bufferStart, size + 1);
    bufferEnd = bufferStart + size;
  }

  Buffer(const byte_t* data, size_t size)
  {
    bufferStart = buffer = (byte_t*)Memory::alloc(size + 1, _capacity); --_capacity;
    Memory::copy(buffer, data, size);
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
  }

  ~Buffer()
  {
    Memory::free(buffer);
  }

  operator const byte_t*() const {return bufferStart;}
  operator byte_t*() {return bufferStart;}

  Buffer& operator=(const Buffer& other)
  {
    size_t size = other.bufferEnd - other.bufferStart;
    if(size > _capacity)
    {
      Memory::free(buffer);
      buffer = (byte_t*)Memory::alloc(size + 1, _capacity); --_capacity;
    }
    Memory::copy(buffer, other.bufferStart, size + 1);
    bufferStart = buffer;
    bufferEnd = buffer + size;
    return *this;
  }

  void_t append(const byte_t* data, size_t size)
  {
    resize(bufferEnd - bufferStart + size);
    Memory::copy(bufferEnd - size, data, size);
    *bufferEnd = 0;
  }

  void_t resize(size_t size)
  {
    size_t requiredCapacity = bufferEnd - buffer + size;
    if(requiredCapacity > _capacity)
    {
      requiredCapacity = bufferEnd - bufferStart + size;
      if(requiredCapacity > _capacity)
      {
        byte_t* newBuffer = (byte_t*)Memory::alloc(requiredCapacity + 1, _capacity); --_capacity;
        size_t oldSize = bufferEnd - bufferStart;
        Memory::copy(newBuffer, bufferStart, oldSize);
        Memory::free(buffer);
        bufferStart = buffer = newBuffer;
        bufferEnd = newBuffer + oldSize;
      }
      else
      {
        size_t oldSize = bufferEnd - bufferStart;
        Memory::move(buffer, bufferStart, oldSize);
        bufferStart = buffer;
        bufferEnd = buffer + oldSize;
      }
    }
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
  }

  void_t removeFront(size_t size)
  {
    bufferStart += size;
    if(bufferStart >= bufferEnd)
      bufferStart = bufferEnd = buffer;
  }

  void_t removeBack(size_t size)
  {
    bufferEnd -= size;
    if(bufferStart >= bufferEnd)
      bufferStart = bufferEnd = buffer;
    *bufferEnd = 0;
  }

  size_t size() const {return bufferEnd - bufferStart;}
  size_t capacity() const {return _capacity;}

  void_t reserve(size_t capacity)
  {
    if(capacity <= _capacity)
      return;
    byte_t* newBuffer = (byte_t*)Memory::alloc(capacity + 1, _capacity); --_capacity;
    size_t size = bufferEnd - bufferStart;
    Memory::copy(newBuffer, bufferStart, size + 1);
    Memory::free(buffer);
    bufferStart = buffer = newBuffer;
    bufferEnd = newBuffer + size;
  }

  void_t clear()
  {
    if(buffer)
    {
      bufferStart = bufferEnd = buffer;
      *bufferEnd = 0;
    }
  }

  bool_t isEmpty() const {return bufferStart == bufferEnd;}

  void_t swap(Buffer& other)
  {
    byte_t* tmpBuffer = buffer;
    byte_t* tmpBufferStart = bufferStart;
    byte_t* tmpBufferEnd = bufferEnd;
    size_t tmpCapacity = _capacity;
    buffer = other.buffer;
    bufferStart = other.bufferStart;
    bufferEnd = other.bufferEnd;
    _capacity = other._capacity;
    other.buffer = tmpBuffer;
    other.bufferStart = tmpBufferStart;
    other.bufferEnd = tmpBufferEnd;
    other._capacity = tmpCapacity;
  }

  void_t free()
  {
    Memory::free(buffer);
    buffer = 0;
    bufferStart = bufferEnd = (byte_t*)&_capacity;
    _capacity = 0;
  }

private:
  byte_t* buffer;
  byte_t* bufferStart;
  byte_t* bufferEnd;
  size_t _capacity;
};

