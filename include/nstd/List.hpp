
#pragma once

#include <nstd/Memory.hpp>

template<typename T> class List
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

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(Iterator(_end.item->prev));}

  usize size() const {return _size;}
  bool isEmpty() const {return endItem.prev == 0;}

  T& prepend(const T& value) {return insert(_begin, value).item->value;}
  T& append(const T& value) {return insert(_end, value).item->value;}

  void prepend(const List& list){insert(_begin, list);}
  void append(const List& list){insert(_end, list);}

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

  void swap(List& other)
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

  Iterator find(const T& value) const
  {
    for(const Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      if(i->value == value)
        return (Item*)i;
    return _end;
  }

  Iterator insert(const Iterator& position, const T& value)
  {
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

    new(item) Item(value);
    freeItem = item->prev;

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

  Iterator insert(const Iterator& position, const List& list)
  {
    if (list.endItem.prev == 0)
      return position;
    Iterator result = insert(position, list._begin.item->value);
    for(const Item* i = list._begin.item->next, * end = &list.endItem; i != end; i = i->next)
      insert(position, i->value);
    return result;
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

  void remove(const T& value)
  {
    Iterator it = find(value);
    if(it != _end)
      remove(it);
  }

  bool operator==(const List& other) const
  {
    if(_size != other._size)
      return false;
    for(const Item* a = _begin.item, * b = other._begin.item; a != &endItem; a = a->next, b = b->next)
      if(a->value != b->value)
        return false;
    return true;
  }

  bool operator!=(const List& other) const {return !(*this == other);}

  /**
  * Sort the list elements.
  */
  void sort()
  {
    if(endItem.prev == 0 || _begin.item == endItem.prev)
      return;
    struct QuickSort
    {
      inline static void swap(Item* a, Item* b)
      {
        T tmp = a->value;
        a->value = b->value;
        b->value = tmp;
      }
      inline static void sort(Item* left, Item* right)
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

    Item() : value() {}

    Item(const T& value) : value(value) {}
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
