#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <iterator>
#include <initializer_list>
#include <stdexcept>


template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> class HashMap
{
public:

    // 9.1 iterator

    class iterator
    {
    public:
        iterator() = default;

        iterator(std::vector<std::list<std::pair<const KeyType, ValueType>>>& table, size_t list_index, typename std::list<std::pair<const KeyType, ValueType>>::iterator inter_it);

        std::pair<const KeyType, ValueType>& operator*();

        std::pair<const KeyType, ValueType>* operator->();

        iterator& operator++();

        iterator operator++(int notused);

        bool operator==(const iterator& other) const;

        bool operator!=(const iterator& other) const;

    private:
        std::vector<std::list<std::pair<const KeyType, ValueType>>>* table_;
        size_t list_index_;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator inter_it_;
    };

    iterator begin();

    iterator end();

    // 9.2 const_iterator

    class const_iterator
    {
    public:
        const_iterator() = default;

        const_iterator(const std::vector<std::list<std::pair<const KeyType, ValueType>>>& table, size_t list_index, typename std::list<std::pair<const KeyType, ValueType>>::const_iterator inter_it);

        const std::pair<const KeyType, ValueType>& operator*();

        const std::pair<const KeyType, ValueType>* operator->();

        const_iterator& operator++();

        const_iterator operator++(int notused);

        bool operator==(const const_iterator& other) const;

        bool operator!=(const const_iterator& other) const;

    private:
        const std::vector<std::list<std::pair<const KeyType, ValueType>>>* table_;
        size_t list_index_;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator inter_it_;
    };

    const_iterator begin() const;

    const_iterator end() const;

    // 1. Конструктор по умолчанию.

    explicit HashMap(const Hash& hash_func = Hash());

    HashMap(const HashMap& other);

    HashMap& operator=(const HashMap& other);

    // 2. Конструктор, принимающий итераторы на начало и конец

    template <class input_iterator>
    HashMap(input_iterator begin, input_iterator end, const Hash& hash_func = Hash());

    // 3. Конструктор, принимающий std::initializer_list

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, const Hash& hash_func = Hash());

    // 4.

    // 5. Методы size и empty, которые должны быть константными

    size_t size() const;

    bool empty() const;

    // 6. Константный метод hash_function

    Hash hash_function() const;

    // 7. Метод insert

    void insert(std::pair<KeyType, ValueType> elem);

    // 8. Метод erase

    void erase(KeyType key);

    // 10. Метод find, константный (возвращающий const_iterator) и нет (возвращающий iterator)

    iterator find(KeyType key);

    const_iterator find(KeyType key) const;

    // 11. Оператор [ ]

    ValueType& operator[](KeyType key);

    // 12. Константный метод at

    const ValueType& at(KeyType key) const;

    // 13. Метод clear

    void clear();

protected:

    void rehash();

private:

    size_t size_, capacity_;

    size_t neighborhood_ = 6;
    double load_factor_ = 0.8;

    Hash hash_func_;

    std::vector<std::list<std::pair<const KeyType, ValueType>>> table_;
};


// 1. Конструктор по умолчанию.


template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash& hash_func) : size_(0), capacity_(24), hash_func_(hash_func) {
    table_.resize(capacity_);
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap& other) : size_(0), capacity_(24), hash_func_(other.hash_func_)
{
    table_.resize(capacity_);
    for (auto elem : other) {
        insert(elem);
    }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType, Hash>::operator=(const HashMap& other)
{
    if (this == &other) {
        return *this;
    }
    clear();
    hash_func_ = other.hash_func_;
    for (auto& elem : other) {
        insert(elem);
    }
    return *this;
}

// 2. Конструктор, принимающий итераторы на начало и конец

template<class KeyType, class ValueType, class Hash>
template<class input_iterator>
HashMap<KeyType, ValueType, Hash>::HashMap(input_iterator begin, input_iterator end, const Hash& hash_func) :
        size_(0), capacity_(24), hash_func_(hash_func)
{
    table_.resize(capacity_);
    while (begin != end) {
        insert(*begin);
        ++begin;
    }
}

// 3. Конструктор, принимающий std::initializer_list

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, const Hash& hash_func) :
        size_(0), capacity_(24), hash_func_(hash_func)
{
    table_.resize(capacity_);
    for (auto& elem : list) {
        insert(elem);
    }
}

// 5. Методы size и empty, которые должны быть константными

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const
{
    return size_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const
{
    return size_ == 0;
}

// 6. Константный метод hash_function

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const
{
    return hash_func_;
}

// 7. Метод insert

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(std::pair<KeyType, ValueType> elem)
{
    if (find(elem.first) != end()) {
        return;
    }
    if (size_ * 1.0 >= capacity_ * load_factor_) {
        rehash();
    }
    size_t index = hash_func_(elem.first) % capacity_;
    size_ += 1;
    for (size_t i = 0; i < neighborhood_; ++i) {
        if (table_[(index + i) % capacity_].empty()) {
            table_[(index + i) % capacity_].push_back(elem);
            return;
        }
    }
    table_[index].push_back(elem);
}

// 8. Метод erase

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(KeyType key)
{
    if (find(key) == end()) {
        return;
    }
    size_t index = hash_func_(key) % capacity_;
    size_ -= 1;
    for (size_t i = 0; i < neighborhood_; ++i) {
        if (!table_[(index + i) % capacity_].empty() && table_[(index + i) % capacity_].front().first == key) {
            table_[(index + i) % capacity_].pop_front();
            break;
        }
    }
    for (typename std::list<std::pair<const KeyType, ValueType>>::iterator it = table_[index].begin(); it != table_[index].end(); ++it) {
        if (it->first == key) {
            table_[index].erase(it);
            break;
        }
    }
}

// 10. Метод find, константный (возвращающий const_iterator) и нет (возвращающий iterator)

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key)
{
    size_t index = hash_func_(key) % capacity_;
    for (size_t i = 0; i < neighborhood_; ++i) {
        if (!table_[(index + i) % capacity_].empty() && table_[(index + i) % capacity_].front().first == key) {
            return iterator(table_, (index + i) % capacity_, table_[(index + i) % capacity_].begin());
        }
    }
    for (auto it = table_[index].begin(); it != table_[index].end(); ++it) {
        if (it->first == key) {
            return iterator(table_, index, it);
        }
    }
    return end();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::find(KeyType key) const
{
    size_t index = hash_func_(key) % capacity_;
    for (size_t i = 0; i < neighborhood_; ++i) {
        if (!table_[(index + i) % capacity_].empty() && table_[(index + i) % capacity_].front().first == key) {
            return const_iterator(table_, (index + i) % capacity_, table_[(index + i) % capacity_].begin());
        }
    }
    for (auto it = table_[index].begin(); it != table_[index].end(); ++it) {
        if (it->first == key) {
            return const_iterator(table_, index, it);
        }
    }
    return end();
}

// 11. Оператор [ ]

template<class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](KeyType key)
{
    iterator it = find(key);
    if (it == end()) {
        insert({ key, ValueType() });
    }
    it = find(key);
    return it->second;
}

// 12. Константный метод at

template<class KeyType, class ValueType, class Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(KeyType key) const
{
    const_iterator it = find(key);
    if (it == end()) {
        throw std::out_of_range("This key does not exist");
    }
    return it->second;
}

// 13. Метод clear

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear()
{
    size_ = 0;
    capacity_ = 24;
    std::vector<std::list<std::pair<const KeyType, ValueType>>> temp_table(capacity_);
    std::swap(table_, temp_table);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::rehash()
{
    capacity_ *= 2;
    std::vector<std::list<std::pair<const KeyType, ValueType>>> temp_table(capacity_);
    std::swap(table_, temp_table);
    size_ = 0;
    for (auto& list : temp_table) {
        for (auto& elem : list) {
            insert(elem);
        }
    }
    return;
}

// 9.1 iterator

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::iterator::iterator(
        std::vector<std::list<std::pair<const KeyType, ValueType>>>& table, size_t list_index,
        typename std::list<std::pair<const KeyType, ValueType>>::iterator inter_it) :
        table_(&table), list_index_(list_index), inter_it_(inter_it) { }

template<class KeyType, class ValueType, class Hash>
std::pair<const KeyType, ValueType>& HashMap<KeyType, ValueType, Hash>::iterator::operator*()
{
    return *inter_it_;
}

template<class KeyType, class ValueType, class Hash>
std::pair<const KeyType, ValueType>* HashMap<KeyType, ValueType, Hash>::iterator::operator->()
{
    return inter_it_.operator->();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator& HashMap<KeyType, ValueType, Hash>::iterator::operator++()
{
    if (list_index_ == table_->size() - 1 && inter_it_ == (*table_)[list_index_].end()) {
        throw std::out_of_range("invalid iterator");
    }
    ++inter_it_;
    if (inter_it_ != (*table_)[list_index_].end() || list_index_ == table_->size() - 1) {
        return *this;
    }
    ++list_index_;
    while ((*table_)[list_index_].empty() && list_index_ != table_->size() - 1) {
        ++list_index_;
    }
    inter_it_ = (*table_)[list_index_].begin();
    return *this;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::iterator::operator++(int notused)
{
    iterator temp = *this;
    ++(*this);
    return temp;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::iterator::operator==(const iterator& other) const
{
    return table_ == other.table_ && list_index_ == other.list_index_ && inter_it_ == other.inter_it_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::iterator::operator!=(const iterator& other) const
{
    return !(*this == other);
}


template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin()
{
    size_t index = 0;
    while (table_[index].empty() && index != capacity_ - 1) {
        ++index;
    }
    return iterator(table_, index, table_[index].begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator HashMap<KeyType, ValueType, Hash>::end()
{
    return iterator(table_, capacity_ - 1, table_.back().end());
}

// 9.2 const_iterator

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::const_iterator::const_iterator(
        const std::vector<std::list<std::pair<const KeyType, ValueType>>>& table, size_t list_index,
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator inter_it) :
        table_(&table), list_index_(list_index), inter_it_(inter_it) { }

template<class KeyType, class ValueType, class Hash>
const std::pair<const KeyType, ValueType>& HashMap<KeyType, ValueType, Hash>::const_iterator::operator*()
{
    return *inter_it_;
}

template<class KeyType, class ValueType, class Hash>
const std::pair<const KeyType, ValueType>* HashMap<KeyType, ValueType, Hash>::const_iterator::operator->()
{
    return inter_it_.operator->();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator& HashMap<KeyType, ValueType, Hash>::const_iterator::operator++()
{
    if (list_index_ == table_->size() - 1 && inter_it_ == (*table_)[list_index_].end()) {
        throw std::out_of_range("invalid iterator");
    }
    ++inter_it_;
    if (inter_it_ != (*table_)[list_index_].end() || list_index_ == table_->size() - 1) {
        return *this;
    }
    ++list_index_;
    while ((*table_)[list_index_].empty() && list_index_ != table_->size() - 1) {
        ++list_index_;
    }
    inter_it_ = (*table_)[list_index_].begin();
    return *this;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::const_iterator::operator++(int notused)
{
    const_iterator temp = *this;
    ++(*this);
    return temp;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::const_iterator::operator==(const const_iterator& other) const
{
    return table_ == other.table_ && list_index_ == other.list_index_ && inter_it_ == other.inter_it_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::const_iterator::operator!=(const const_iterator& other) const
{
    return !(*this == other);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::begin() const
{
    size_t index = 0;
    while (table_[index].empty() && index != capacity_ - 1) {
        ++index;
    }
    return const_iterator(table_, index, table_[index].begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator HashMap<KeyType, ValueType, Hash>::end() const
{
    return const_iterator(table_, capacity_ - 1, table_.back().end());
}
