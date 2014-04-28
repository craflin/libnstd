
#pragma once

#include <nstd/Memory.h>

template<typename T, typename V> class HashMap
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& key() const {return item->key;}
    V& operator*() {return item->value;}
    const V& operator*() const {return item->value;}
    const V* operator->() const {return &item->value;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class HashMap;
  };

  HashMap() : _end(&endItem), _begin(&endItem), _size(0), capacity(500), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  HashMap(const HashMap& other) : _end(&endItem), _begin(&endItem), _size(0), capacity(500), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->key, i->value);
  }

  explicit HashMap(size_t capacity) : _end(&endItem), _begin(&endItem), _size(0), capacity(capacity), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    _size |= (size_t)!_size;
  }

  ~HashMap()
  {
    if(data)
      Memory::free(data);
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      i->~Item();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      Memory::free(i);
    }
  }

  HashMap& operator=(const HashMap& other)
  {
    clear();
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->key, i->value);
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const { return _begin.item->value; }
  const T& back() const { return _end.item->prev->value; }

  V& front() { return _begin.item->value; }
  V& back() { return _end.item->prev->value; }

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  size_t size() const {return _size;}
  bool_t isEmpty() const {return endItem.prev == 0;}

  V& prepend(const T& key, const V& value) {return insert(_begin, key, value).item->value;}
  V& append(const T& key, const V& value) {return insert(_end, key, value).item->value;}

  void_t clear()
  {
    for(Item* i = _begin.item, * end = &endItem, * next; i != end; i = next)
    {
      next = i->next;
      i->~Item();
      *i->cell = 0;
      i->prev = freeItem;
      freeItem = i;
    }
    _begin.item = &endItem;
    endItem.prev = 0;
    _size = 0;
  }

  void_t swap(HashMap& other)
  {
    Item* tmpFirst = _begin.item;
    Item* tmpLast = endItem.prev;
    size_t tmpSize = _size;
    size_t tmpCapacity = capacity;
    Item** tmpData = data;
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
    capacity = other.capacity;
    data = other.data;
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
    other.capacity = tmpCapacity;
    other.data = tmpData;
    other.freeItem = tmpFreeItem;
    other.blocks = tmpBlocks;
  }

  Iterator find(const T& key) const
  {
    if(!data) return _end;
    size_t hashCode = (size_t)key;
    Item* item = data[hashCode % capacity];
    while(item)
    {
      if(item->key == key) return item;
      item = item->nextCell;
    }
    return _end;
  }

  bool_t contains(const T& key) const {return find(key) != _end;}

  Iterator insert(const Iterator& position, const T& key, const V& value)
  {
    Iterator it = find(key);
    if(it != _end)
    {
      *it = value;
      return it;
    }

    if(!data)
    {
      size_t size;
      data = (Item**)Memory::alloc(sizeof(Item*) * capacity, size);
      capacity = size / sizeof(Item*);
      Memory::zero(data, sizeof(Item*) * capacity);
    }
    
    Item* item;
    if(freeItem)
    {
      item = freeItem;
      freeItem = freeItem->prev;
    }
    else
    {
      size_t allocatedSize;
      ItemBlock* itemBlock = (ItemBlock*)Memory::alloc(sizeof(ItemBlock) + sizeof(Item), allocatedSize);
      itemBlock->next = blocks;
      blocks = itemBlock;
      item = (Item*)((char_t*)itemBlock + sizeof(ItemBlock));

      for(Item* i = item + 1, * end = item + (allocatedSize - sizeof(ItemBlock)) / sizeof(Item); i < end; ++i)
      {
        i->prev = freeItem;
        freeItem = i;
      }
    }

    size_t hashCode = (size_t)key;
#ifdef VERIFY
    VERIFY(new(item) Item(key, value) == item);
#else
    new(item) Item(key, value);
#endif

    Item** cell;
    item->cell = (cell = &data[hashCode % capacity]);
    if((item->nextCell = *cell))
      item->nextCell->cell = &item->nextCell;
    *cell = item;
    
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

    if((*item->cell = item->nextCell))
      item->nextCell->cell = item->cell;

    if(!item->prev)
      (_begin.item = item->next)->prev = 0;
    else
      (item->prev->next = item->next)->prev = item->prev;

    --_size;

    item->~Item();
    item->prev = freeItem;
    freeItem = item;
    return item->next;
  }

  void_t remove(const T& key)
  {
    Iterator it = find(key);
    if(it != _end)
      remove(it);
  }
  
private:
  struct Item
  {
    const T key;
    V value;
    Item** cell;
    Item* nextCell;
    Item* prev;
    Item* next;

    Item() : key(), value() {}

    Item(const T& key, const V& value) : key(key), value(value) {}
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

  Iterator _end;
  Iterator _begin;
  size_t _size;
  size_t capacity;
  Item** data;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;
};
