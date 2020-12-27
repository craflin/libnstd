
#pragma once

#include <nstd/Memory.hpp>

class Buffer
{
public:
  Buffer() : buffer(0), bufferStart((byte*)&_capacity), bufferEnd(bufferStart), _capacity(0) {}

  explicit Buffer(usize capacity) 
  {
    bufferStart = bufferEnd = buffer = (byte*)Memory::alloc(capacity + 1, _capacity); --_capacity;
    *bufferEnd = 0;
  }

  Buffer(const Buffer& other)
  {
    usize size = other.bufferEnd - other.bufferStart;
    bufferStart = buffer = (byte*)Memory::alloc(size + 1, _capacity); --_capacity;
    Memory::copy(buffer, other.bufferStart, size);
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
  }

  Buffer(const byte* data, usize size)
  {
    bufferStart = buffer = (byte*)Memory::alloc(size + 1, _capacity); --_capacity;
    Memory::copy(buffer, data, size);
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
  }

  ~Buffer()
  {
    Memory::free(buffer);
  }

  void attach(byte* data, usize length)
  { 
    Memory::free(buffer);
    buffer = 0;
    bufferStart = data;
    bufferEnd = data + length;
  }

  operator const byte*() const {return bufferStart;}
  operator byte*() {return bufferStart;}

  Buffer& operator=(const Buffer& other)
  {
    usize size = other.bufferEnd - other.bufferStart;
    if(size > _capacity)
    {
      Memory::free(buffer);
      buffer = (byte*)Memory::alloc(size + 1, _capacity); --_capacity;
    }
    else if(!buffer)
      return *this;
    Memory::copy(buffer, other.bufferStart, size);
    bufferStart = buffer;
    bufferEnd = buffer + size;
    *bufferEnd = 0;
    return *this;
  }

  void assign(const byte* data, usize size)
  {
    if(size > _capacity)
    {
      Memory::free(buffer);
      buffer = (byte*)Memory::alloc(size + 1, _capacity); --_capacity;
    }
    else if(!buffer)
      return;
    Memory::copy(buffer, data, size);
    bufferStart = buffer;
    bufferEnd = buffer + size;
    *bufferEnd = 0;
  }

  bool operator==(const Buffer& other) const
  {
    usize size = bufferEnd - bufferStart;
    return size == (usize)(other.bufferEnd - other.bufferStart) && Memory::compare(other.bufferStart, bufferStart, size) == 0;
  }

  bool operator!=(const Buffer& other) const
  {
    usize size = bufferEnd - bufferStart;
    return size != (usize)(other.bufferEnd - other.bufferStart) || Memory::compare(other.bufferStart, bufferStart, size) != 0;
  }

  void prepend(const byte* data, usize size)
  {
    if(buffer && (usize)(bufferStart - buffer) >= size)
    {
      bufferStart -= size;
      Memory::copy(bufferStart, data, size);
      return;
    }
    usize oldSize = bufferEnd - bufferStart;
    usize requiredCapacity = size + oldSize;
    if(buffer && _capacity >= requiredCapacity)
    {
      Memory::move(buffer + size, bufferStart, oldSize);
      Memory::copy(buffer, data, size);
      bufferStart = buffer;
      bufferEnd = buffer + requiredCapacity;
    }
    else
    {
      byte* newBuffer = (byte*)Memory::alloc(requiredCapacity + 1, _capacity); --_capacity;
      Memory::copy(newBuffer, data, size);
      Memory::copy(newBuffer + size, bufferStart, oldSize);
      Memory::free(buffer);
      bufferStart = buffer = newBuffer;
      bufferEnd = newBuffer+ requiredCapacity;
    }
  }

  void prepend(const Buffer& data) {prepend(data, data.size());}

  void append(const byte* data, usize size)
  {
    resize(bufferEnd - bufferStart + size);
    Memory::copy(bufferEnd - size, data, size);
    *bufferEnd = 0;
  }

  void append(const Buffer& data)
  {
    usize size = data.bufferEnd - data.bufferStart;
    resize(bufferEnd - bufferStart + size);
    Memory::copy(bufferEnd - size, data.bufferStart, size);
    *bufferEnd = 0;
  }

  void resize(usize size)
  {
    if(size > _capacity)
    {
      byte* newBuffer = (byte*)Memory::alloc(size + 1, _capacity); --_capacity;
      Memory::copy(newBuffer, bufferStart, bufferEnd - bufferStart);
      Memory::free(buffer);
      bufferStart = buffer = newBuffer;
      bufferEnd = newBuffer + size;
      *bufferEnd = 0;
    }
    else if(buffer)
    {
      if(bufferStart + size <= buffer + _capacity)
      {
        bufferEnd = bufferStart + size;
        *bufferEnd = 0;
      }
      else
      {
        Memory::move(buffer, bufferStart, bufferEnd - bufferStart);
        bufferStart = buffer;
        bufferEnd = buffer + size;
        *bufferEnd = 0;
      }
    }
  }

  void removeFront(usize size)
  {
    bufferStart += size;
    if(bufferStart >= bufferEnd)
      bufferStart = bufferEnd = buffer ? buffer : (byte*)&_capacity;
  }

  void removeBack(usize size)
  {
    if(bufferStart + size >= bufferEnd)
      bufferStart = bufferEnd = buffer ? buffer : (byte*)&_capacity;
    else
      bufferEnd -= size;
    *bufferEnd = 0;
  }

  usize size() const {return bufferEnd - bufferStart;}
  usize capacity() const {return _capacity;}

  void reserve(usize capacity)
  {
    if(capacity <= _capacity)
      return;
    byte* newBuffer = (byte*)Memory::alloc(capacity + 1, _capacity); --_capacity;
    usize size = bufferEnd - bufferStart;
    Memory::copy(newBuffer, bufferStart, size);
    Memory::free(buffer);
    bufferStart = buffer = newBuffer;
    bufferEnd = newBuffer + size;
    *bufferEnd = 0;
  }

  void clear()
  {
    if(buffer)
    {
      bufferStart = bufferEnd = buffer;
      *bufferEnd = 0;
    }
    else
      bufferEnd = bufferStart;
  }

  bool isEmpty() const {return bufferStart == bufferEnd;}

  void swap(Buffer& other)
  {
    byte* tmpBuffer = buffer;
    byte* tmpBufferStart = bufferStart;
    byte* tmpBufferEnd = bufferEnd;
    usize tmpCapacity = _capacity;
    buffer = other.buffer;
    bufferStart = other.bufferStart;
    bufferEnd = other.bufferEnd;
    _capacity = other._capacity;
    other.buffer = tmpBuffer;
    other.bufferStart = tmpBufferStart;
    other.bufferEnd = tmpBufferEnd;
    other._capacity = tmpCapacity;
  }

  void free()
  {
    Memory::free(buffer);
    buffer = 0;
    bufferStart = bufferEnd = (byte*)&_capacity;
    _capacity = 0;
  }

private:
  byte* buffer;
  byte* bufferStart;
  byte* bufferEnd;
  usize _capacity;
};
