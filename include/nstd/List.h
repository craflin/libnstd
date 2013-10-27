
#pragma once

#include <nstd/Memory.h>

template<typename T> class List
{
private:
  struct Item;
public:
  class Iterator
  {
  public:
    Iterator() : item(0) {}
    T& operator*() {return item->value;}
    const T& operator*() const {return item->value;}
    const Iterator& operator++() {item = item->next; return *this;}
    const Iterator& operator--() {item = item->prev; return *this;}
    bool operator==(const Iterator& other) const {return item == other.item;}
    bool operator!=(const Iterator& other) const {return item != other.item;}

  private:
    Item* item;
    
    Iterator(Item* item) : item(item) {}

    friend class List;
  };

  List() : _size(0), freeItem(0), blocks(0), _end(&endItem), _begin(&endItem)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  ~List()
  {
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
  
  Iterator find(const T& value)
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      if(i->value == value)
        return i;
    return _end;
  }
  
  void_t append(const T& value)
  {
    Item* item;
    if(freeItem)
    {
      item = freeItem;
      freeItem = freeItem->next;
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
        i->next = freeItem;
        freeItem = i;
      }
    }

    item->Item::Item();
    item->value = value;
    
    if((item->prev = endItem.prev))
      endItem.prev->next = item;
    else
      _begin.item = item;

    item->next = &endItem;
    endItem.prev = item;
    ++_size;
  }

  void_t remove(const Iterator& it)
  {
    Item* item = it.item;

    if(!item->prev)
      (_begin.item = item->next)->prev = 0;
    else
      (item->prev->next = item->next)->prev = item->prev;

    --_size;

    item->~Item();
    item->next = freeItem;
    freeItem = item;
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
    T value;
    Item* prev;
    Item* next;
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

  Item endItem;
  Iterator _end;
  Iterator _begin;
  size_t _size;
  Item* freeItem;
  ItemBlock* blocks;
};
