
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
    const T& key() {return item->key;}
    V& operator*() {return item->value;}
    const V& operator*() const {return item->value;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class HashMap;
  };

  HashMap() : _end(&endItem), _begin(&endItem), _size(0), capacity(0), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  explicit HashMap(size_t capacity) : _end(&endItem), _begin(&endItem), _size(0), capacity(capacity), data(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
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

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  size_t size() const {return _size;}
  bool_t isEmpty() const {return endItem.prev == 0;}

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

  Iterator find(const T& key)
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
  
  V& insert(const T& key, const V& value)
  {
    Iterator it = find(key);
    if(it != _end)
    {
      V& v = *it;
      v = value;
      return v;
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
    VERIFY(new(item) Item(key, value) == item);
    //item->Item::Item(key, value);
    Item** cell;
    item->cell = (cell = &data[hashCode % capacity]);
    item->nextCell = *cell;
    *cell = item;
    
    if((item->prev = endItem.prev))
      endItem.prev->next = item;
    else
      _begin.item = item;

    item->next = &endItem;
    endItem.prev = item;
    ++_size;
    return item->value;
  }

  Iterator remove(const Iterator& it)
  {
    Item* item = it.item;

    *item->cell = item->nextCell;

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

    Item() : key() {}

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
