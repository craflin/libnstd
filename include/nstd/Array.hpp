
#pragma once

#include <nstd/Memory.hpp>

template <typename T> class Array
{
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& operator*() const {return *item;}
    T& operator*() {return *item;}
    const T* operator->() const {return item;}
    T* operator->() {return item;}
    const Iterator& operator++() {++item; return *this;}
    const Iterator& operator--() {--item; return *this;}
    Iterator operator++() const {return item + 1;}
    Iterator operator--() const {return item - 1;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    T* item;
    
    Iterator(T* item) : item(item) {}

    friend class Array;
  };

  Array() : _capacity(0) {}

  explicit Array(usize capacity) : _capacity(capacity) {}

  Array(const Array& other) : _capacity(0)
  {
    reserve(other.capacity());
    T* dest = _begin.item;
    for(T* src = other._begin.item, * end = other._end.item; src != end; ++src, ++dest)
    {
#ifdef VERIFY
      VERIFY(new(dest)T(*src) == dest);
#else
      new(dest)T(*src);
#endif
    }
    _end.item = dest;
  }

  ~Array()
  {
    if(_begin.item)
    {
      for(T* i = _begin.item, * end = _end.item; i != end; ++i)
        i->~T();
      Memory::free(_begin.item);
    }
  }

  Array& operator=(const Array& other)
  {
    clear();
    reserve(other.capacity());
    T* dest = _begin.item;
    for(T* src = other._begin.item, * end = other._end.item; src != end; ++src, ++dest)
    {
#ifdef VERIFY
      VERIFY(new(dest)T(*src) == dest);
#else
      new(dest)T(*src);
#endif
    }
    _end.item = dest;
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const {return *_begin.item;}
  const T& back() const {return _end.item[-1];}

  T& front() {return *_begin.item;}
  T& back() {return _end.item[-1];}

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item - 1);}

  operator const T*() const {return _begin.item;}

  operator T*() {return _begin.item;}

  usize size() const {return _end.item - _begin.item;}
  bool isEmpty() const {return _begin.item == _end.item;}

  void reserve(usize size)
  {
    if(size > _capacity || (!_begin.item && size > 0))
    {
      usize bsize;
      if (size > _capacity)
        _capacity = size;
      T* newData = (T*)Memory::alloc(sizeof(T) * _capacity, bsize);
      _capacity = bsize / sizeof(T);
      T* dest = newData;
      if(_begin.item)
      {
        for (T* src = _begin.item, * end = _end.item; src != end; ++src, ++dest)
        {
#ifdef VERIFY
          VERIFY(new(dest)T(*src) == dest);
#else
          new(dest)T(*src);
#endif
          src->~T();
        }
        Memory::free(_begin.item);
      }
      _begin.item = newData;
      _end.item = dest;
    }
  }

  void resize(usize size, const T& value = T())
  {
    usize _size = _end.item - _begin.item;
    if (size < _size)
    {
      T* newEnd = _begin.item + size;
      for (T* i = newEnd, *end = _end.item; i != end; ++i)
        i->~T();
      _end.item = newEnd;
    }
    else
    {
      reserve(size);
      T* end = _begin.item + size;
      for (T* i = _begin.item + _size; i != end; ++i)
      {
#ifdef VERIFY
        VERIFY(new(i)T(value) == i);
#else
        new(i)T(value);
#endif
      }
      _end.item = end;
    }
  }

  usize capacity() const {return _capacity;}

  void clear()
  {
    if(_begin.item)
    {
      for(T* i = _begin.item, * end = _end.item; i != end; ++i)
        i->~T();
      _end.item = _begin.item;
    }
  }

  void swap(Array& other)
  {
    T* tmpFirst = _begin.item;
    T* tmpEnd = _end.item;
    usize tmpCapacity = _capacity;

    _begin.item = other._begin.item;
    _end.item = other._end.item;
    _capacity = other._capacity;

    other._begin.item = tmpFirst;
    other._end.item = tmpEnd;
    other._capacity = tmpCapacity;
  }

  T& append(const T& value)
  {
    usize size = _end.item - _begin.item;
    reserve(size + 1);
    T* item = _end.item;
#ifdef VERIFY
    VERIFY(new(item) T(value) == item);
#else
    new(item) T(value);
#endif
    ++_end.item;
    return *item;
  }

  void append(const Array& values)
  {
    usize size = _end.item - _begin.item;
    usize valuesSize = values.size();
    reserve(size + valuesSize);
    T* item = _end.item;
    for(T* end = item + valuesSize, *src = values._begin.item; item < end; ++item, ++src)
    {
#ifdef VERIFY
      VERIFY(new(item) T(*src) == item);
#else
      new(item) T(*src);
#endif
    }
    _end.item = item;
  }

  void append(const T* values, usize size)
  {
    usize oldSize = _end.item - _begin.item;
    reserve(oldSize + size);
    T* item = _end.item;
    for(T* end = item + size; item < end; ++item, ++values)
    {
#ifdef VERIFY
      VERIFY(new(item) T(*values) == item);
#else
      new(item) T(*values);
#endif
    }
    _end.item = item;
  }

  void remove(usize index)
  {
    usize size = _end.item - _begin.item;
    if(index < size)
    {
      T* pos = _begin.item + index;
      for(T* end = --_end.item, * dest; pos < end;)
      {
        dest = pos;
        *dest = *(++pos);
      }
      pos->~T();
    }
  }

  Iterator remove(const Iterator& it)
  {
    T* pos = it.item;
    for(T* end = --_end.item, * dest; pos < end;)
    {
      dest = pos;
      *dest = *(++pos);
    }
    pos->~T();
    return it.item;
  }

  Iterator find(const T& value) const
  {
    for(T* pos = _begin.item, * end = _end.item; pos < end; ++pos)
      if(*pos == value)
        return pos;
    return _end;
  }

private:
  Iterator _begin;
  Iterator _end;
  usize _capacity;
};

