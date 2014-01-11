
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

  List() : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
  }

  List(const List& other) : _end(&endItem), _begin(&endItem), _size(0), freeItem(0), blocks(0)
  {
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->value);
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

  List& operator=(const List& other)
  {
    clear();
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      append(i->value);
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const T& front() const { return _begin.item->value; }
  const T& back() const { return _end.item->prev->value; }

  T& front() { return _begin.item->value; }
  T& back() { return _end.item->prev->value; }

  size_t size() const {return _size;}
  bool_t isEmpty() const {return endItem.prev == 0;}

  T& prepend(const T& value) {return insert(_begin, value).item->value;}
  T& append(const T& value) {return insert(_end, value).item->value;}

  void_t clear()
  {
    for(Item* i = _begin.item, * end = &endItem, * next; i != end; i = next)
    {
      next = i->next;
      i->~Item();
      i->prev = freeItem;
      freeItem = i;
    }
    _begin.item = &endItem;
    endItem.prev = 0;
    _size = 0;
  }

  Iterator find(const T& value)
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      if(i->value == value)
        return i;
    return _end;
  }

  Iterator insert(const Iterator& position, const T& value)
  {
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

#ifdef VERIFY
    VERIFY(new(item) Item(value) == item);
#else
    new(item) Item(value);
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

  /**
  * Sort the list elements.
  */
  void_t sort()
  {
    if(endItem.prev == 0)
      return;
    struct QuickSort
    {
      inline static void_t swap(Item* a, Item* b)
      {
        T tmp = a->value;
        a->value = b->value;
        b->value = tmp;
      }
      inline static void_t sort(Item* left, Item* right)
      {
        Item* ptr0, * ptr1, * ptr2;
        ptr0 = ptr1 = ptr2 = left;
        const T& pivot = left->value;
        do
        {
          ptr2 = ptr2->next;
          if(ptr2->value < pivot)
          {
            ptr0 = ptr1;
            ptr1 = ptr1->next;
            swap(ptr1, ptr2);
          }
        } while(ptr2 != right);
        swap(left, ptr1);
        if(ptr1 != right)
          ptr1 = ptr1->next;
        if(left != ptr0)
          sort(left, ptr0);
        if(ptr1 != right)
          sort(ptr1, right);
      }
    };
    QuickSort::sort(_begin.item, endItem.prev);
  }

private:
  struct Item
  {
    T value;
    Item* prev;
    Item* next;

    Item() {}

    Item(const T& value) : value(value) {}
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
