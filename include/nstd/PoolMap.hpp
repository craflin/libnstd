
#pragma once

#include <nstd/Memory.hpp>

template<typename T, typename V> class PoolMap
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& key() const {return item->key;}
    const V& operator*() const {return item->value;}
    V& operator*() {return item->value;}
    const V* operator->() const {return &item->value;}
    V* operator->() {return &item->value;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    Iterator operator++() const {return item->next;}
    Iterator operator--() const {return item->prev;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class PoolMap;
  };

  PoolMap() : _end(&endItem), _begin(&endItem), _size(0), capacity(500), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  explicit PoolMap(usize capacity) : _end(&endItem), _begin(&endItem), _size(0), capacity(capacity), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    this->capacity |= (usize)!capacity;
  }

  ~PoolMap()
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

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const { return _begin.item->value; }
  const T& back() const { return _end.item->prev->value; }

  V& front() { return _begin.item->value; }
  V& back() { return _end.item->prev->value; }

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  usize size() const {return _size;}
  bool isEmpty() const {return endItem.prev == 0;}

  V& append(const T& key) {return insert(_end, key).item->value;}

  void clear()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
    {
      i->~Item();
      *i->cell = 0;
      i->prev = freeItem;
      freeItem = i;
    }
    _begin.item = &endItem;
    endItem.prev = 0;
    _size = 0;
  }

  void swap(PoolMap& other)
  {
    Item* tmpFirst = _begin.item;
    Item* tmpLast = endItem.prev;
    usize tmpSize = _size;
    usize tmpCapacity = capacity;
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
    usize hashCode = hash(key);
    Item* item = data[hashCode % capacity];
    while(item)
    {
      if(item->key == key) return item;
      item = item->nextCell;
    }
    return _end;
  }

  bool contains(const T& key) const {return find(key) != _end;}

  Iterator insert(const Iterator& position, const T& key)
  {
    Iterator it = find(key);
    if(it != _end)
      return it;

    if(!data)
    {
      usize size;
      data = (Item**)Memory::alloc(sizeof(Item*) * capacity, size);
      capacity = size / sizeof(Item*);
      Memory::zero(data, sizeof(Item*) * capacity);
    }
    
    Item* item = freeItem;
    if(!item)
    {
      usize allocatedSize;
      ItemBlock* itemBlock = (ItemBlock*)Memory::alloc(sizeof(ItemBlock) + sizeof(Item), allocatedSize);
      itemBlock->next = blocks;
      blocks = itemBlock;
      for(Item* i = (Item*)(itemBlock + 1), * end = i + (allocatedSize - sizeof(ItemBlock)) / sizeof(Item); i < end; ++i)
      {
        i->prev = item;
        item = i;
      }
      freeItem = item;
    }

    usize hashCode = hash(key);
    new(item) Item(key);
    freeItem = item->prev;

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
    remove(item->value);
    return item->next;
  }

  void remove(const V& value)
  {
    Item* item = (Item*)&value;

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
  }

  void remove(const T& key)
  {
    Iterator it = find(key);
    if(it != _end)
      remove(it.item->value);
  }
  
private:
  struct Item
  {
    V value;
    const T key;
    Item** cell;
    Item* nextCell;
    Item* prev;
    Item* next;

    Item() : value(), key() {}

    Item(const T& key) : value(), key(key) {}
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

  Iterator _end;
  Iterator _begin;
  usize _size;
  usize capacity;
  Item** data;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;

  PoolMap(const PoolMap&);
  PoolMap& operator=(const PoolMap&);
};
