
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
    Memory::copy(buffer, other.bufferStart, size);
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
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

  void_t attach(byte_t* data, size_t length)
  {
    Memory::free(buffer);
    buffer = 0;
    bufferStart = data;
    bufferEnd = data + length;
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
    else if(!buffer)
      return *this;
    Memory::copy(buffer, other.bufferStart, size);
    bufferStart = buffer;
    bufferEnd = buffer + size;
    *bufferEnd = 0;
    return *this;
  }

  void_t assign(const byte_t* data, size_t size)
  {
    if(size > _capacity)
    {
      Memory::free(buffer);
      buffer = (byte_t*)Memory::alloc(size + 1, _capacity); --_capacity;
    }
    else if(!buffer)
      return;
    Memory::copy(buffer, data, size);
    bufferStart = buffer;
    bufferEnd = buffer + size;
    *bufferEnd = 0;
  }

  bool_t operator==(const Buffer& other) const
  {
    size_t size = bufferEnd - bufferStart;
    return size == (size_t)(other.bufferEnd - other.bufferStart) && Memory::compare(other.bufferStart, bufferStart, size) == 0;
  }

  bool_t operator!=(const Buffer& other) const
  {
    size_t size = bufferEnd - bufferStart;
    return size != (size_t)(other.bufferEnd - other.bufferStart) || Memory::compare(other.bufferStart, bufferStart, size) != 0;
  }

  void_t prepend(const byte_t* data, size_t size)
  {
    if((size_t)(bufferStart - buffer) >= size)
    {
      bufferStart -= size;
      Memory::copy(bufferStart, data, size);
      return;
    }
    size_t oldSize = bufferEnd - bufferStart;
    size_t requiredCapacity = size + oldSize;
    if(_capacity >= requiredCapacity)
    {
      Memory::move(buffer + size, bufferStart, oldSize);
      Memory::copy(buffer, data, size);
      bufferStart = buffer;
      bufferEnd = buffer + requiredCapacity;
    }
    else
    {
      byte_t* newBuffer = (byte_t*)Memory::alloc(requiredCapacity + 1, _capacity); --_capacity;
      Memory::copy(newBuffer, data, size);
      Memory::copy(newBuffer + size, bufferStart, oldSize);
      Memory::free(buffer);
      bufferStart = buffer = newBuffer;
      bufferEnd = newBuffer+ requiredCapacity;
    }
  }

  void_t prepend(const Buffer& data) {prepend(data, data.size());}

  void_t append(const byte_t* data, size_t size)
  {
    resize(bufferEnd - bufferStart + size);
    Memory::copy(bufferEnd - size, data, size);
    *bufferEnd = 0;
  }

  void_t append(const Buffer& data)
  {
    size_t size = data.bufferEnd - data.bufferStart;
    resize(bufferEnd - bufferStart + size);
    Memory::copy(bufferEnd - size, data.bufferStart, size);
    *bufferEnd = 0;
  }

  void_t resize(size_t size)
  {
    if(size > _capacity)
    {
      byte_t* newBuffer = (byte_t*)Memory::alloc(size + 1, _capacity); --_capacity;
      Memory::copy(newBuffer, bufferStart, bufferEnd - bufferStart);
      Memory::free(buffer);
      bufferStart = buffer = newBuffer;
      bufferEnd = newBuffer + size;
      *bufferEnd = 0;
    }
    else if(bufferStart + size <= buffer + _capacity)
    {
      bufferEnd = bufferStart + size;
      *bufferEnd = 0;
    }
    else if(buffer)
    {
      Memory::move(buffer, bufferStart, bufferEnd - bufferStart);
      bufferStart = buffer;
      bufferEnd = buffer + size;
      *bufferEnd = 0;
    }
  }

  void_t removeFront(size_t size)
  {
    bufferStart += size;
    if(bufferStart >= bufferEnd)
      bufferStart = bufferEnd = buffer ? buffer : (byte_t*)&_capacity;
  }

  void_t removeBack(size_t size)
  {
    if(bufferStart + size >= bufferEnd)
      bufferStart = bufferEnd = buffer ? buffer : (byte_t*)&_capacity;
    else
      bufferEnd -= size;
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
    Memory::copy(newBuffer, bufferStart, size);
    Memory::free(buffer);
    bufferStart = buffer = newBuffer;
    bufferEnd = newBuffer + size;
    *bufferEnd = 0;
  }

  void_t clear()
  {
    if(buffer)
    {
      bufferStart = bufferEnd = buffer;
      *bufferEnd = 0;
    }
    else
      bufferEnd = bufferStart;
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
