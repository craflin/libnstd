
#pragma once

#include <nstd/Memory.hpp>

template<typename T> class HashSet
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    const T& operator*() const {return item->key;}
    const T* operator->() const {return &item->key;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    Iterator operator++() const {return item->next;}
    Iterator operator--() const {return item->prev;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class HashSet;
  };

  HashSet() : _end(&endItem), _begin(&endItem), _size(0), capacity(500), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  HashSet(const HashSet& other) : _end(&endItem), _begin(&endItem), _size(0), capacity(500), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->key);
  }

  explicit HashSet(usize capacity) : _end(&endItem), _begin(&endItem), _size(0), capacity(capacity), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    this->capacity |= (usize)!capacity;
  }

  ~HashSet()
  {
    delete[] (char*)data;
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      i->~Item();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      delete[] (char*)i;
    }
  }

  HashSet& operator=(const HashSet& other)
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

  usize size() const {return _size;}
  bool isEmpty() const {return endItem.prev == 0;}

  void prepend(const T& key) {insert(_begin, key);}
  void append(const T& key) {insert(_end, key);}

  void append(const HashSet& other)
  {
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      insert(_end, i->key);
  }

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

  void swap(HashSet& other)
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
    if (it != _end) return it;

    if (!data)
    {
      data = (Item**)new char[sizeof(Item*) * capacity];
      Memory::zero(data, sizeof(Item*)* capacity);
    }

    Item* item;
    if (freeItem)
    {
      item = freeItem;
      freeItem = freeItem->prev;
    }
    else
    {
      ItemBlock* itemBlock = (ItemBlock*)new char[sizeof(ItemBlock) + sizeof(Item) * 4];
      itemBlock->next = blocks;
      blocks = itemBlock;
      item = (Item*)((char*)itemBlock + sizeof(ItemBlock));

      for (Item* i = item + 1, *end = item + 4; i < end; ++i)
      {
        i->prev = freeItem;
        freeItem = i;
      }
    }

    usize hashCode = hash(key);
#ifdef VERIFY
    VERIFY(new(item)Item(key) == item);
#else
    new(item)Item(key);
#endif

    Item** cell;
    item->cell = (cell = &data[hashCode % capacity]);
    if((item->nextCell = *cell))
      item->nextCell->cell = &item->nextCell;
    *cell = item;

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

  void remove(const T& key)
  {
    Iterator it = find(key);
    if(it != _end)
      remove(it);
  }

  void remove(const HashSet& other)
  {
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      remove(i->key);
  }

  bool operator==(const HashSet& other) const
  {
    if(_size != other._size)
      return false;
    for(const Item* a = _begin.item, * b = other._begin.item; a != &endItem; a = a->next, b = b->next)
      if(a->key != b->key)
        return false;
    return true;
  }

  bool operator!=(const HashSet& other) const {return !(*this == other);}

private:
  struct Item
  {
    const T key;
    Item** cell;
    Item* nextCell;
    Item* prev;
    Item* next;

    Item() : key() {}

    Item(const T& key) : key(key) {}
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
};
