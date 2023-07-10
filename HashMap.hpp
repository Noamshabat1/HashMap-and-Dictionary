#ifndef HASH_MAP_HPP
#define HASH_MAP_HPP

#define DEFAULT_CAPACITY 16
#define CAPACITY_CHANGER 2
#define LOWER_BOUND ((double)1 / 4)
#define UPPER_BOUND ((double)3 / 4)
#define ITERATOR_END_VALUE -1

// Messages
#define LENGTH_ERROR_KEYS_VALUES_SIZE "The length of Keys and Values lists do not match!"
#define VALUE_WITH_THAT_KEY_DOESNT_EXIST "A value with the given key does not exist!"

#include <algorithm>
#include <stdexcept>
#include <vector>

template <class KeyT, class ValueT>
class HashMap
{
  public:
    HashMap()
    {
      initialize(DEFAULT_CAPACITY, 0);
    }

    HashMap(const std::vector<KeyT>& keys, const std::vector<ValueT>& values)
        : HashMap()
    {
      if (keys.size() != values.size())
      {
        throw std::length_error(LENGTH_ERROR_KEYS_VALUES_SIZE);
      }

      for (size_t i = 0; i < keys.size(); i++)
      {
        insert(keys[i], values[i], true);
      }

      resizeUp();
    }

    HashMap(const HashMap<KeyT, ValueT>& hash_map)
        : HashMap()
    {
      *this = hash_map;
    }

    virtual ~HashMap()
    {
      delete[] data;
    }

    // Getters
    int size() const
    {
      return mapSize;
    }

    int capacity() const
    {
      return mapCapacity;
    }

    bool empty() const
    {
      return size() == 0;
    }

    bool containsKey(const KeyT& key) const
    {
      const entry* ent = findEntryByKey(key);
      return (ent != nullptr);
    }

    int getBucketIndex(const KeyT& key) const
    {
      if (!containsKey(key))
      {
        throw std::invalid_argument(VALUE_WITH_THAT_KEY_DOESNT_EXIST);
      }

      return hash(key);
    }

    int getBucketSize(const KeyT& key) const
    {
      if (!containsKey(key))
      {
        throw std::invalid_argument(VALUE_WITH_THAT_KEY_DOESNT_EXIST);
      }

      return data[hash(key)].size();
    }

    double getLoadFactor() const
    {
      return static_cast<double>(size()) / capacity();
    }

    // Functions
    bool insert(const KeyT& key, const ValueT& value)
    {
      bool result = insertEntry(key, value);

      if (!result)
      {
        return false;
      }

      resizeUp();
      return true;
    }

    const ValueT& at(const KeyT& key) const
    {
      const entry* ent = findEntryByKey(key);

      if (ent == nullptr)
      {
        throw std::invalid_argument(VALUE_WITH_THAT_KEY_DOESNT_EXIST);
      }

      return ent->second;
    }

    ValueT& at(const KeyT& key)
    {
      entry* ent = findEntryByKey(key);

      if (ent == nullptr)
      {
        throw std::invalid_argument(VALUE_WITH_THAT_KEY_DOESNT_EXIST);
      }

      return ent->second;
    }

    bool erase(const KeyT& key)
    {
      entry* ent = findEntryByKey(key);

      if (ent == nullptr)
      {
        return false;
      }

      bucket& buck = data[hash(key)];
      buck.erase(std::remove(buck.begin(), buck.end(), *ent), buck.end());
      mapSize--;

      resizeDown();
      return true;
    }

    void clear()
    {
      delete[] data;
      initialize(mapCapacity, 0);
    }

    // Operators
    HashMap<KeyT, ValueT>& operator=(const HashMap<KeyT, ValueT>& hash_map)
    {
      if (*this == hash_map)
      {
        return *this;
      }

      if (mapCapacity != hash_map.mapCapacity)
      {
        delete[] data;
        initialize(hash_map.mapCapacity, 0);
      }

      mapSize = hash_map.mapSize;
      for (int i = 0; i < mapCapacity; i++)
      {
        data[i] = hash_map.data[i];
      }

      return *this;
    }

    ValueT& operator[](const KeyT& key)
    {
      entry* ent = findEntryByKey(key);

      if (ent != nullptr)
      {
        return ent->second;
      }

      ValueT val = ValueT();
      insert(key, val);

      return findEntryByKey(key)->second;
    }

    const ValueT& operator[](const KeyT& key) const
    {
      return at(key);
    }

    friend bool operator==(const HashMap<KeyT, ValueT>& a, const HashMap<KeyT, ValueT>& b)
    {
      if (a.mapSize != b.mapSize)
      {
        return false;
      }

      for (const entry& item : a)
      {
        if (!b.containsKey(item.first) || b.at(item.first) != item.second)
        {
          return false;
        }
      }

      for (const entry& item : b)
      {
        if (!a.containsKey(item.first) || a.at(item.first) != item.second)
        {
          return false;
        }
      }

      return true;
    };

    friend bool operator!=(const HashMap<KeyT, ValueT>& a, const HashMap<KeyT, ValueT>& b)
    {
      return !(a == b);
    };

    // Iterator
    class ConstIterator;
    typedef ConstIterator const_iterator;

    ConstIterator begin() const
    {
      return ConstIterator(this);
    }

    ConstIterator end() const
    {
      return ConstIterator(this, ITERATOR_END_VALUE, ITERATOR_END_VALUE);
    }

    ConstIterator cbegin() const
    {
      return ConstIterator(this);
    }

    ConstIterator cend() const
    {
      return ConstIterator(this, ITERATOR_END_VALUE, ITERATOR_END_VALUE);
    }

  protected:
    typedef std::pair<KeyT, ValueT> entry;
    typedef std::vector<entry> bucket;

    void resizeUp()
    {
      int optimalCapacity = calculateOptimalCapacityUp(size());

      if (mapCapacity >= optimalCapacity)
      {
        return;
      }

      resize(optimalCapacity);
    }

    void resizeDown()
    {
      int optimalCapacity = calculateOptimalCapacityDown(size());

      if (mapCapacity <= optimalCapacity)
      {
        return;
      }

      resize(optimalCapacity);
    }

    void resize(int optimalCapacity)
    {
      int oldCapacity = mapCapacity;
      bucket* oldData = data;

      initialize(optimalCapacity, 0);

      for (int i = 0; i < oldCapacity; i++)
      {
        bucket& buck = oldData[i];

        for (entry& ent : buck)
        {
          insertEntry(ent.first, ent.second);
        }
      }

      delete[] oldData;
    }

    entry* findEntryByKey(const KeyT& key)
    {
      int index = hash(key);

      bucket& buck = data[index];
      for (entry& ent : buck)
      {
        if (ent.first == key)
        {
          return &ent;
        }
      }

      return nullptr;
    }

    const entry* findEntryByKey(const KeyT& key) const
    {
      int index = hash(key);

      bucket& buck = data[index];
      for (const entry& ent : buck)
      {
        if (ent.first == key)
        {
          return &ent;
        }
      }

      return nullptr;
    }

    bool insertEntry(const KeyT& key, const ValueT& value, bool shouldReplace = false)
    {
      entry* ent = findEntryByKey(key);

      if (ent != nullptr)
      {
        if (shouldReplace)
        {
          ent->second = value;
          return true;
        }

        return false;
      }

      int index = hash(key);

      bucket& buck = data[index];
      buck.emplace_back(key, value);

      mapSize++;
      return true;
    }

  private:
    std::hash<KeyT> hashFunction;

    int mapCapacity;
    int mapSize;

    bucket* data;

    void initialize(int capacity = DEFAULT_CAPACITY, int size = 0)
    {
      hashFunction = std::hash<KeyT>{};

      mapCapacity = capacity;
      mapSize = size;
      data = new bucket[mapCapacity];

      for (int i = 0; i < mapCapacity; i++)
      {
        data[i] = bucket(0);
      }
    }

    int calculateOptimalCapacityUp(long M) const
    {
      if (M == 0)
      {
        return 1;
      }

      double factor = static_cast<double>(M) / mapCapacity;

      if (factor <= UPPER_BOUND)
      {
        return mapCapacity;
      }

      return mapCapacity * 2;
    }

    int calculateOptimalCapacityDown(long M) const
    {
      if (M == 0)
      {
        return 1;
      }

      int cap = 1;
      double factor = static_cast<double>(M) / cap;

      while (factor >= LOWER_BOUND)
      {
        cap *= 2;
        factor = static_cast<double>(M) / cap;
      }

      return cap / 2;
    }

    int hash(const KeyT& key) const
    {
      return hashFunction(key) & (mapCapacity - 1);
    }
};

template <class KeyT, class ValueT>
class HashMap<KeyT, ValueT>::ConstIterator
{
  public:
    typedef entry value_type;
    typedef const entry& reference;
    typedef const entry* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::forward_iterator_tag iterator_category;

    ConstIterator(const HashMap<KeyT, ValueT>* hash_map)
        : _map(hash_map)
    {
      setToAvailableBucket(0);
    }

    ConstIterator(const HashMap<KeyT, ValueT>* hash_map, int bucket_index, int entry_index)
        : _map(hash_map)
    {
      this->bucket_index = bucket_index;
      this->entry_index = entry_index;
    }

    reference operator*() const
    {
      return (*_map).data[bucket_index][entry_index];
    }

    pointer operator->() const
    {
      return &(*_map).data[bucket_index][entry_index];
    }

    ConstIterator& operator++()
    {
      entry_index++;

      if (static_cast<unsigned long>((*_map).data[bucket_index].size())
       == static_cast<unsigned long>(entry_index))
      {
        setToAvailableBucket(bucket_index + 1);
      }

      return *this;
    }

    ConstIterator operator++(int)
    {
      ConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const ConstIterator& a, const ConstIterator& b)
    {
      return a._map == b._map && a.bucket_index == b.bucket_index && a.entry_index == b.entry_index;
    }

    friend bool operator!=(const ConstIterator& a, const ConstIterator& b)
    {
      return !(a == b);
    }

  private:
    const HashMap<KeyT, ValueT>* _map;
    int bucket_index, entry_index;

    void setToAvailableBucket(int starting_pos)
    {
      for (int i = starting_pos; i < (*_map).mapCapacity; i++)
      {
        bucket& buck = (*_map).data[i];
        if (!buck.empty())
        {
          bucket_index = i;
          entry_index = 0;
          return;
        }
      }

      bucket_index = ITERATOR_END_VALUE;
      entry_index = ITERATOR_END_VALUE;
    }
};

#endif
