
#pragma once

#include <nstd/Debug.h>
#include <nstd/Memory.h>

template<typename T, typename V> class Map
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

    friend class Map;
  };
  
public:
  Map() : _end(&endItem), _begin(&endItem), root(0), _size(0), freeItem(0), blocks(0)
  {
    endItem.parent = 0;
    endItem.prev = 0;
    endItem.next = 0;
  }

  Map(const Map& other) : _end(&endItem), _begin(&endItem), root(0), _size(0), freeItem(0), blocks(0)
  {
    endItem.parent = 0;
    endItem.prev = 0;
    endItem.next = 0;
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      insert(i->key, i->value);
  }

  ~Map()
  {
    for(Item* i = _begin.item, * end = &endItem; i != end; i = i->next)
      i->~Item();
    for(ItemBlock* i = blocks, * next; i; i = next)
    {
      next = i->next;
      Memory::free(i);
    }
  }

  Map& operator=(const Map& other)
  {
    clear();
    for(const Item* i = other._begin.item, * end = &other.endItem; i != end; i = i->next)
      insert(i->key, i->value);
    return *this;
  }

  const Iterator& begin() const {return _begin;}
  const Iterator& end() const {return _end;}

  const V& front() const {return _begin.item->value;}
  const V& back() const {return _end.item->prev->value;}

  V& front() {return _begin.item->value;}
  V& back() {return _end.item->prev->value;}

  Iterator removeFront() {return remove(_begin);}
  Iterator removeBack() {return remove(_end.item->prev);}

  usize size() const {return _size;}
  bool isEmpty() const {return !root;}

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
    root = 0;
  }

  Iterator find(const T& key) const
  {
    for(Item* item = root; item; )
    {
      if(key > item->key)
      {
        item = item->right;
        continue;
      }
      else if(key < item->key)
      {
        item = item->left;
        continue;
      }
      else
        return item;
    }
    return _end;
  }

  bool contains(const T& key) const {return find(key) != _end;}

  Iterator insert(const Iterator& position, const T& key, const V& value)
  {
    Item* insertPos = position.item;
    if(insertPos == &endItem)
    {
      Item* prev = insertPos->prev;
      if(prev && key > prev->key)
        return insert(&prev->right, prev, key, value);
    }
    else
    {
      if(key < insertPos->key)
      {
        Item* prev = insertPos->prev;
        if(!prev || key > prev->key)
          return insert(&insertPos->left, insertPos, key, value);
      }
      else if(key > insertPos->key)
      {
        Item* next = insertPos->next;
        if(next == &endItem || key < next->key)
          return insert(&insertPos->right, insertPos, key, value);
      }
      else
      {
        insertPos->value = value;
        return insertPos;
      }
    }
    return insert(&root, 0, key, value);
  }
  
  Iterator insert(const T& key, const V& value)
  {
    return insert(&root, 0, key, value);
  }

  //void checkTree(Item* item, Item* parent)
  //{
  //  ASSERT(item->parent == parent);
  //  usize height = item->height;
  //  ssize slope = item->slope;
  //  if(item->left)
  //    checkTree(item->left, item);
  //  if(item->right)
  //    checkTree(item->right, item);
  //  usize leftHeight = item->left ? item->left->height : 0;
  //  usize rightHeight = item->right ? item->right->height : 0;
  //  ASSERT(height == (leftHeight > rightHeight ? leftHeight : rightHeight) + 1);
  //  ASSERT(slope == leftHeight - rightHeight);
  //  ASSERT(slope <= 1);
  //  ASSERT(slope >= -1);
  //}

  void remove(const T& key)
  {
    Iterator it = find(key);
    if(it != _end)
      remove(it);
  }

  Iterator remove(const Iterator& it)
  {
    Item* item = it.item;
    Item* origParent;
    Item* parent = origParent = item->parent;
    Item** cell = parent ? (item == parent->left ? &parent->left : &parent->right) : &root;
    ASSERT(*cell == item);
    Item* left = item->left;
    Item* right = item->right;

    if(!left && !right)
    {
      *cell = 0;
      goto rebalParentUpwards;
    }

    if(!left)
    {
      *cell = right;
      right->parent = parent;
      goto rebalParentUpwards;
    }

    if(!right)
    {
      *cell = left;
      left->parent = parent;
      goto rebalParentUpwards;
    }

    if(left->height < right->height)
    {
      Item* next = item->next;
      ASSERT(!next->left);
      Item* nextParent = next->parent;
      if(nextParent == item)
      {
        ASSERT(next == right);
        *cell = next;
        next->parent = parent;

        next->left = left;
        left->parent = next;
        parent = next;
        goto rebalParent;
      }

      // unlink next
      ASSERT(next == nextParent->left);
      Item* nextRight = next->right;
      nextParent->left = nextRight;
      if(nextRight)
        nextRight->parent = nextParent;
      
      // put next in cell
      *cell = next;
      next->parent = parent;

      // link left to next left
      next->left = left;
      left->parent = next;

      // link right to next right
      next->right = right;
      right->parent = next;

      // rebal old next->parent
      parent = nextParent;
      ASSERT(parent);
      goto rebalParent;
    }
    else
    {
      Item* prev = item->prev;
      ASSERT(!prev->right);
      Item* prevParent = prev->parent;
      if(prevParent == item)
      {
        ASSERT(prev == left);
        *cell = prev;
        prev->parent = parent;

        prev->right = right;
        right->parent = prev;
        parent = prev;
        goto rebalParent;
      }

      // unlink prev
      ASSERT(prev == prevParent->right);
      Item* prevLeft = prev->left;
      prevParent->right = prevLeft;
      if(prevLeft)
        prevLeft->parent = prevParent;

      // put prev in cell
      *cell = prev;
      prev->parent = parent;

      // link right to prev right
      prev->right = right;
      right->parent = prev;

      // link left to prev left
      prev->left = left;
      left->parent = prev;

      // rebal old prev->parent
      parent = prevParent;
      ASSERT(parent);
      goto rebalParent;
    }

    // rebal parent upwards
  rebalParent:
    do
    {
      usize oldHeight = parent->height;
      parent->updateHeightAndSlope();
      parent = rebal(parent);
      if(oldHeight == parent->height)
      {
        parent = *cell;
        parent->updateHeightAndSlope();
        parent = rebal(parent);
        parent = parent->parent;
        break;
      }
      parent = parent->parent;
    } while(parent != origParent);
  rebalParentUpwards:
    while(parent)
    {
      usize oldHeight = parent->height;
      parent->updateHeightAndSlope();
      parent = rebal(parent);
      if(oldHeight == parent->height)
        break;
      parent = parent->parent;
    }

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

private:
  struct Item
  {
    const T key;
    V value;
    Item* parent;
    Item* left;
    Item* right;
    usize height;
    ssize slope;
    Item* next;
    Item* prev;
    
    Item() : key(), value() {}
    
    Item(Item* parent, const T& key, const V& value) : key(key), value(value), parent(parent), left(0), right(0), height(1), slope(0) {}
    
    void updateHeightAndSlope()
    {
      ASSERT(!parent || this == parent->left || this == parent->right);
      usize leftHeight = left ? left->height : 0;
      usize rightHeight = right ? right->height : 0;
      slope = leftHeight - rightHeight;
      height = (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
    }
  };
  struct ItemBlock
  {
    ItemBlock* next;
  };

private:
  Iterator _end;
  Iterator _begin;
  Item* root;
  usize _size;
  Item endItem;
  Item* freeItem;
  ItemBlock* blocks;

private:
  Iterator insert(Item** cell, Item* parent, const T& key, const V& value)
  {
  begin:
    ASSERT(!parent || cell == &parent->left || cell == &parent->right);
    Item* position = *cell;
    if(!position)
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
      new(item) Item(parent, key, value);

      *cell = item;
      ++_size;
      if(!parent)
      { // first item
        ASSERT(_begin.item == &endItem);
        item->prev = 0;
        item->next = _begin.item;
        _begin.item = item;
        endItem.prev = item;
      }
      else
      {
        Item* insertPos = cell == &parent->right ? 
          parent->next : // insert after parent
          parent; // insert before parent
        if((item->prev = insertPos->prev))
          insertPos->prev->next = item;
        else
          _begin.item = item;
        item->next = insertPos;
        insertPos->prev = item;

        // neu:
        usize oldHeight;
        do
        {
          oldHeight = parent->height;
          parent->updateHeightAndSlope();
          parent = rebal(parent);
          if(oldHeight == parent->height)
            break;
          parent = parent->parent;
        } while(parent);
      }
      return item;
    }
    else
    {
      if(key > position->key)
      {
        cell = &position->right;
        parent = position;
        goto begin;
      }
      else if(key < position->key)
      {
        cell = &position->left;
        parent = position;
        goto begin;
      }
      else
      {
        position->value = value;
        return position;
      }
    }
  }
  
  Item* rebal(Item* item)
  {
    if(item->slope > 1)
    {
      ASSERT(item->slope == 2);
      Item* parent = item->parent;
      Item*& cell = parent ? (parent->left == item ? parent->left : parent->right) : root;
      ASSERT(cell == item);
      shiftr(cell);
      ASSERT((cell->slope < 0 ? -cell->slope : cell->slope) <= 1);
      return cell;
    }
    else if(item->slope < -1)
    {
      ASSERT(item->slope == -2);
      Item* parent = item->parent;
      Item*& cell = parent ? (parent->left == item ? parent->left : parent->right) : root;
      ASSERT(cell == item);
      shiftl(cell);
      ASSERT((cell->slope < 0 ? -cell->slope : cell->slope) <= 1);
      return cell;
    }
    return item;
  }
  
  static void rotr(Item*& cell)
  {
    Item* oldTop = cell;
    Item* result = oldTop->left;
    Item* tmp = result->right;
    result->parent = oldTop->parent;
    result->right = oldTop;
    if((oldTop->left = tmp))
      tmp->parent = oldTop;
    oldTop->parent = result;
    cell = result;
    oldTop->updateHeightAndSlope();
    result->updateHeightAndSlope();
  }
  
  static void rotl(Item*& cell)
  {
    Item* oldTop = cell;
    Item* result = oldTop->right;
    Item* tmp = result->left;
    result->parent = oldTop->parent;
    result->left = oldTop;
    if((oldTop->right = tmp))
      tmp->parent = oldTop;
    oldTop->parent = result;
    cell = result;
    oldTop->updateHeightAndSlope();
    result->updateHeightAndSlope();
  }
  
  static void shiftr(Item*& cell)
  {
    Item* oldTop = cell;
    if(oldTop->left->slope == -1)
      rotl(oldTop->left);
    rotr(cell);
  }

  static void shiftl(Item*& cell)
  {
    Item* oldTop = cell;
    if(oldTop->right->slope == 1)
      rotr(oldTop->right);
    rotl(cell);
  }
};
