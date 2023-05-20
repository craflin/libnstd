
#pragma once

#include <nstd/Memory.hpp>

class Buffer
{
public:
  Buffer() : buffer(0), bufferStart((byte*)&_capacity), bufferEnd(bufferStart), _capacity(0) {}

  explicit Buffer(usize capacity)
      : _capacity(capacity)
  {
    bufferStart = bufferEnd = buffer = (byte*)new char[capacity + 1];
    *bufferEnd = 0;
  }

  Buffer(const Buffer& other)
      : _capacity(other.bufferEnd - other.bufferStart)
  {
    bufferStart = buffer = (byte*)new char[_capacity + 1];
    Memory::copy(buffer, other.bufferStart, _capacity);
    bufferEnd = bufferStart + _capacity;
    *bufferEnd = 0;
  }

  Buffer(const byte* data, usize size)
      : _capacity(size)
  {
    bufferStart = buffer = (byte*)new char[size + 1];
    Memory::copy(buffer, data, size);
    bufferEnd = bufferStart + size;
    *bufferEnd = 0;
  }

  ~Buffer()
  {
    delete[] (char*)buffer;
  }

  void attach(byte* data, usize length)
  { 
    delete[] (char*)buffer;
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
      delete[] (char*)buffer;
      _capacity = size;
      buffer = (byte*)new char[size + 1];
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
      delete[] (char*)buffer;
      _capacity = size;
      buffer = (byte*)new char[size + 1];
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
        _capacity = requiredCapacity;
      byte* newBuffer = (byte*)new char[requiredCapacity + 1];
      Memory::copy(newBuffer, data, size);
      Memory::copy(newBuffer + size, bufferStart, oldSize);
      delete [] (char*)buffer;
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
        _capacity = size;
      byte* newBuffer = (byte*)new char[size + 1];
      Memory::copy(newBuffer, bufferStart, bufferEnd - bufferStart);
      delete[] (char*)buffer;
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
    _capacity = capacity;
    byte* newBuffer = (byte*)new char [capacity + 1];
    usize size = bufferEnd - bufferStart;
    Memory::copy(newBuffer, bufferStart, size);
    delete[] (char*)buffer;
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
    delete[] (char*)buffer;
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
