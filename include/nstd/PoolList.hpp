
#pragma once

#include <nstd/Memory.hpp>

template<typename T> class PoolList
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& operator*() const {return *(T*)(item + 1);}
    T& operator*() {return *(T*)(item + 1);}
    const T* operator->() const {return (T*)(item + 1);}
    T* operator->() {return (T*)(item + 1);}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    Iterator operator++() const {return item->next;}
    Iterator operator--() const {return item->prev;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class PoolList;
  };

  PoolList() : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  ~PoolList()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      ((T*)(i + 1))->~T();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      Memory::free(i);
    }
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const {return _begin.item->value;}
  const T& back() const {return _end.item->prev->value;}

  T& front() { return _begin.item->value; }
  T& back() { return _end.item->prev->value; }

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  usize size() const {return _size;}
  bool isEmpty() const {return endItem.prev == 0;}

  T& append() {return linkFreeItem(new (allocateFreeItem()) T);}
  template<typename A>
  T& append(A a) {return linkFreeItem(new (allocateFreeItem()) T(a));}
  template<typename A, typename B>
  T& append(A a, B b) {return linkFreeItem(new (allocateFreeItem()) T(a, b));}
  template<typename A, typename B, typename C>
  T& append(A a, B b, C c) {return linkFreeItem(new (allocateFreeItem()) T(a, b, c));}
  template<typename A, typename B, typename C, typename D>
  T& append(A a, B b, C c, D d) {return linkFreeItem(new (allocateFreeItem()) T(a, b, c, d));}
  template<typename A, typename B, typename C, typename D, typename E>
  T& append(A a, B b, C c, D d, E e) {return linkFreeItem(new (allocateFreeItem()) T(a, b, c, d, e));}
  template<typename A, typename B, typename C, typename D, typename E, typename F>
  T& append(A a, B b, C c, D d, E e, F f) {return linkFreeItem(new (allocateFreeItem()) T(a, b, c, d, e, f));}
  template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
  T& append(A a, B b, C c, D d, E e, F f, G g) {return linkFreeItem(new (allocateFreeItem()) T(a, b, c, d, e, f, g));}

  void clear()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
    {
      ((T*)(i + 1))->~T();
      i->prev = freeItem;
      freeItem = i;
    }
    _begin.item = &endItem;
    endItem.prev = 0;
    _size = 0;
  }

  void swap(PoolList& other)
  {
    Item* tmpFirst = _begin.item;
    Item* tmpLast = endItem.prev;
    usize tmpSize = _size;
    Item* tmpFreeItem = freeItem;
    ItemBlock* tmpBlocks = blocks;

    if((endItem.prev = other.endItem.prev))
    {
      endItem.prev->next = &endItem;
      _begin.item = other._begin.item;
    }
    else
      _begin.item = &endItem;
    _size = other._size;
    freeItem = other.freeItem;
    blocks = other.blocks;

    if((other.endItem.prev = tmpLast))
    {
      tmpLast->next = &other.endItem;
      other._begin.item = tmpFirst;
    }
    else
      other._begin.item = &other.endItem;
    other._size = tmpSize;
    other.freeItem = tmpFreeItem;
    other.blocks = tmpBlocks;
  }

  Iterator remove(const Iterator& it)
  {
    Item* item = it.item;
    remove(*(T*)(item + 1));
    return item->next;
  }

  void remove(const T& value)
  {
    Item* item = (Item*)&value - 1;

    if(!item->prev)
      (_begin.item = item->next)->prev = 0;
    else
      (item->prev->next = item->next)->prev = item->prev;

    --_size;

    ((T*)(item + 1))->~T();
    item->prev = freeItem;
    freeItem = item;
  }

private:
  struct Item
  {
    Item* prev;
    Item* next;
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

private:
  Iterator _end;
  Iterator _begin;
  usize _size;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;

private:
  T* allocateFreeItem()
  {
    Item* item = freeItem;
    if(!item)
    {
      usize allocatedSize;
      ItemBlock* itemBlock = (ItemBlock*)Memory::alloc(sizeof(ItemBlock) + sizeof(Item) + sizeof(T), allocatedSize);
      itemBlock->next = blocks;
      blocks = itemBlock;
      for(Item* i = (Item*)(itemBlock + 1), * end = (Item*)((char*)i + (allocatedSize - sizeof(ItemBlock)) / (sizeof(Item) + sizeof(T)) * (sizeof(Item) + sizeof(T)));
        i < end; 
        i = (Item*)((char*)i + (sizeof(Item) + sizeof(T))))
      {
        i->prev = item;
        item = i;
      }
      freeItem = item;
    }
    return (T*)(item + 1);
  }

  T& linkFreeItem(T* t)
  {
    Item* item = freeItem;
    freeItem = item->prev;
    Item* insertPos = _end.item;
    if((item->prev = insertPos->prev))
      insertPos->prev->next = item;
    else
      _begin.item = item;
    item->next = insertPos;
    insertPos->prev = item;
    ++_size;
    return *t;
  }

private:
  PoolList(const PoolList&);
  PoolList& operator=(const PoolList&);
};
