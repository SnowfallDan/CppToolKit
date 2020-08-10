#ifndef CPPTOOLKITS_LRUQUEUE_H
#define CPPTOOLKITS_LRUQUEUE_H

#include <string>
#include <map>
#include <list>
#include <unordered_map>
#include <memory>

#include "disableCopyAndAssign.h"

using std::list;
using std::string;

template<typename K, typename V>
class Entry
{
public:
    typedef std::shared_ptr<V> ValuePtr;
    explicit Entry(K key, V *ptr) : key(key), value(ptr) {}

    ~Entry()
    {
        next = nullptr;
        pre = nullptr;
    }

    K key;
    ValuePtr value;
    Entry *next = nullptr;
    Entry *pre = nullptr;
};

template<typename K, typename V>
class List final
{
public:
    typedef Entry<K, V> EntryType;

    // 禁用拷贝构造和等号运算符
    DISABLE_COPY_AND_ASSIGN(List);

    List() = default;

    ~List()
    {
        clear();
    }

    void clear()
    {
        EntryType *temp;
        auto ptr = first_->next;
        while (ptr != last_)
        {
            temp = ptr->next;
            first_->next = temp;
            temp->pre = first_;
            delete ptr;
            ptr = temp;
        }

        delete ptr;
        size_ = 0;
        first_ = nullptr;
        last_ = nullptr;
    }

    template <class...Args>
    EntryType *emplace_front(Args&&... args)
    {
        auto entry = new EntryType(std::forward<Args>(args)...);
        return emplace_front(entry);
    }

    EntryType *emplace_front(EntryType *entry)
    {
        if(size_ == 0)
        {
            first_ = entry;
            last_ = entry;
            first_->next = last_;
            last_->pre = first_;
            size_ = 1;
        }
        else
        {
            entry->pre = last_;
            entry->next = first_;
            first_->pre = entry;
            last_->pre = entry;
            first_ = entry;
            ++size_;
        }
        return entry;
    }

    template <class...Args>
    EntryType *emplace_back(Args&&... args)
    {
        auto entry = new EntryType(std::forward<Args>(args)...);
        return emplace_back(entry);
    }

    EntryType *emplace_back(EntryType *entry)
    {
        if(size_ == 0)
        {
            first_ = entry;
            last_ = entry;
            first_->next = last_;
            last_->pre = first_;
            size_ = 1;
        }
        else
        {
            last_->next = entry;
            entry->pre = last_;
            last_ = entry;
            first_->pre = last_;
            size_++;
        }
    }

    K pop_front()
    {
        auto ptr = first_;
        first_ = first_->next;
        auto key = ptr->key;
        delete ptr;
        if(!first_)
            last_ = nullptr;
        --size_;
        return key;
    }

    K pop_back()
    {
        auto ptr = last_;
        last_ = last_->pre;
        auto key = ptr->key;
        delete ptr;
        if(!last_)
            first_ = nullptr;
        --size_;
        return key;
    }


    void move_to_front(EntryType *entry)
    {
        if(!entry || empty() || size_ == 1 || entry == first_)
             return;
        if(entry == last_)
            entry->pre->next = nullptr;
        else
        {
            entry->pre->next = entry->next;
            entry->next->pre = entry->pre;
        }
        // 下面emplace_front会重新将计数+1
        --size_;
        emplace_front(entry);
    }

    EntryType *front() const
    {
        return first_;
    }

    EntryType *back() const
    {
        return last_;
    }

    uint64_t size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    template <typename FUNCTION>
    void for_each(FUNCTION &&func)
    {
        auto ptr = first_;
        while (ptr)
        {
            func(ptr->value);
            ptr = ptr->next;
        }
    }

private:
    uint64_t size_ = 0;
    EntryType *first_ = nullptr;
    EntryType *last_ = nullptr;
};

template<typename K, typename V>
class LRUQueue final
{
public:
    typedef std::shared_ptr<V> ValuePtr;

    // 禁用拷贝构造和等号运算符
    DISABLE_COPY_AND_ASSIGN(LRUQueue);

    explicit LRUQueue(uint fixedCapacity = 1024) : fixed_capacity_(fixedCapacity) {}

    ~LRUQueue() = default;

    template <class...Args>
    void put(const K &key, Args&&... args)
    {
        auto it = lru_map_.find(key);
        if (it != lru_map_.end())
        {
            // 插入重复元素，只更新位置
            lru_list_.move_to_front(it->second);
            return;
        }

        auto entry = new Entry<K, V>(key, new V(std::forward<Args>(args)...));
        lru_map_[key] = lru_list_.emplace_front(entry);

        if (lru_list_.size() > fixed_capacity_)
            lru_map_.erase(lru_list_.pop_back());
    }

    ValuePtr get(const K &key)
    {
        auto it = lru_map_.find(key);
        if (it == lru_map_.end())
            return nullptr;

        lru_list_.move_to_front(it->second);
        return it->second->value;
    }

private:
    uint fixed_capacity_;
    std::unordered_map<K, Entry<K, V> *> lru_map_;
    List<K, V> lru_list_;
};

#endif
