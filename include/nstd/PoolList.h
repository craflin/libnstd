
#pragma once

#include <nstd/Memory.h>

template<typename T> class PoolList
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& operator*() const {return item->value;}
    T& operator*() {return item->value;}
    const T* operator->() const {return &item->value;}
    T* operator->() {return &item->value;}
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

  PoolList(const PoolList& other) : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->value);
  }

  ~PoolList()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      i->~Item();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      Memory::free(i);
    }
  }

  PoolList& operator=(const PoolList& other)
  {
    clear();
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->value);
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const {return _begin.item->value;}
  const T& back() const {return _end.item->prev->value;}

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  usize size() const {return _size;}
  bool isEmpty() const {return endItem.prev == 0;}

  T& prepend() {return insert(_begin).item->value;}
  T& append() {return insert(_end).item->value;}

  void append(const PoolList& other)
  {
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      insert(_end, i->value);
  }

  void clear()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
    {
      i->~Item();
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

  Iterator insert(const Iterator& position)
  {
    Item* item;
    if(freeItem)
    {
      item = freeItem;
      freeItem = freeItem->prev;
    }
    else
    {
      usize allocatedSize;
      ItemBlock* itemBlock = (ItemBlock*)Memory::alloc(sizeof(ItemBlock) + sizeof(Item), allocatedSize);
      itemBlock->next = blocks;
      blocks = itemBlock;
      item = (Item*)((char*)itemBlock + sizeof(ItemBlock));

      for(Item* i = item + 1, * end = item + (allocatedSize - sizeof(ItemBlock)) / sizeof(Item); i < end; ++i)
      {
        i->prev = freeItem;
        freeItem = i;
      }
    }

#ifdef VERIFY
    VERIFY(new(item) Item == item);
#else
    new(item) Item;
#endif

    Item* insertPos = position.item;
    if((item->prev = insertPos->prev))
      insertPos->prev->next = item;
    else
      _begin.item = item;

    item->next = insertPos;
    insertPos->prev = item;
    ++_size;
    return item;
  }

  Iterator remove(const Iterator& it)
  {
    Item* item = it.item;
    remove(item->value);
    return item->next;
  }

  void remove(const T& value)
  {
    Item* item = (Item*)&value;

    if(!item->prev)
      (_begin.item = item->next)->prev = 0;
    else
      (item->prev->next = item->next)->prev = item->prev;

    --_size;

    item->~Item();
    item->prev = freeItem;
    freeItem = item;
  }

private:
  struct Item
  {
    T value;
    Item* prev;
    Item* next;

    Item() : value() {}
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

  Iterator _end;
  Iterator _begin;
  usize _size;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;
};
