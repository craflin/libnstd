
#pragma once

#include <nstd/Memory.h>

template<typename T> class Pool
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& operator*() const {return item->key;}
    T& operator*() {return item->key;}
    const T* operator->() const {return &item->key;}
    T* operator->() {return &item->key;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    Iterator operator++() const {return item->next;}
    Iterator operator--() const {return item->prev;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class Pool;
  };

  Pool() : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  Pool(const Pool& other) : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->key);
  }

  ~Pool()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      i->~Item();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      Memory::free(i);
    }
  }

  Pool& operator=(const Pool& other)
  {
    clear();
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->key);
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const {return _begin.item->key;}
  const T& back() const {return _end.item->prev->key;}

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  size_t size() const {return _size;}
  bool_t isEmpty() const {return endItem.prev == 0;}

  T& prepend() {return insert(_begin).item->key;}
  T& append() {return insert(_end).item->key;}

  void_t append(const Pool& other)
  {
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      insert(_end, i->key);
  }

  void_t clear()
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

  void_t swap(Pool& other)
  {
    Item* tmpFirst = _begin.item;
    Item* tmpLast = endItem.prev;
    size_t tmpSize = _size;
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
    if (freeItem)
    {
      item = freeItem;
      freeItem = freeItem->prev;
    }
    else
    {
      size_t allocatedSize;
      ItemBlock* itemBlock = (ItemBlock*)Memory::alloc(sizeof(ItemBlock)+sizeof(Item), allocatedSize);
      itemBlock->next = blocks;
      blocks = itemBlock;
      item = (Item*)((char_t*)itemBlock + sizeof(ItemBlock));

      for (Item* i = item + 1, *end = item + (allocatedSize - sizeof(ItemBlock)) / sizeof(Item); i < end; ++i)
      {
        i->prev = freeItem;
        freeItem = i;
      }
    }

#ifdef VERIFY
    VERIFY(new(item)Item == item);
#else
    new(item)Item;
#endif

    Item* insertPos = position.item;
    if ((item->prev = insertPos->prev))
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
    remove(item->key);
    return item->next;
  }

  void_t remove(const T& key)
  {
    Item* item = (Item*)&key;

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
    T key;
    Item* prev;
    Item* next;

    Item() : key() {}
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

  Iterator _end;
  Iterator _begin;
  size_t _size;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;
};