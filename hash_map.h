#pragma once
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

namespace MyHashTable {

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    class iterator;
    class const_iterator;

    // 1. Конструктор по умолчанию.

    explicit HashMap(const Hash &hash_func = Hash())
        : size_(0), capacity_(start_capacity_), hash_func_(hash_func) {
        table_.resize(capacity_);
    }

    HashMap(const HashMap &other)
        : size_(other.size_),
          capacity_(other.capacity_),
          hash_func_(other.hash_func_),
          table_(other.table_) {
    }

    HashMap(HashMap &&other)
        : size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)),
          hash_func_(std::exchange(other.hash_func_, Hash())),
          table_(std::exchange(other.table_, {})) {
    }

    HashMap &operator=(HashMap other) {
        swap(other);
        return *this;
    }

    ~HashMap() = default;

    // 2. Конструктор, принимающий итераторы на начало и конец

    template <class input_iterator>
    HashMap(input_iterator begin, input_iterator end, const Hash &hash_func = Hash())
        : size_(0), capacity_(start_capacity_), hash_func_(hash_func) {
        table_.resize(capacity_);
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    // 3. Конструктор, принимающий std::initializer_list

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list,
            const Hash &hash_func = Hash())
        : size_(0), capacity_(start_capacity_), hash_func_(hash_func) {
        table_.resize(capacity_);
        for (auto &elem : list) {
            insert(elem);
        }
    }

    // 5. Методы size и empty, которые должны быть константными

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    // 6. Константный метод hash_function

    Hash hash_function() const {
        return hash_func_;
    }

    // 7. Метод insert

    void insert(std::pair<KeyType, ValueType> elem) {
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

    void erase(KeyType key) {
        if (find(key) == end()) {
            return;
        }
        size_t index = hash_func_(key) % capacity_;
        size_ -= 1;
        for (size_t i = 0; i < neighborhood_; ++i) {
            if (!table_[(index + i) % capacity_].empty() &&
                table_[(index + i) % capacity_].front().first == key) {
                table_[(index + i) % capacity_].pop_front();
                break;
            }
        }
        for (typename std::list<std::pair<const KeyType, ValueType>>::iterator it =
                 table_[index].begin();
             it != table_[index].end(); ++it) {
            if (it->first == key) {
                table_[index].erase(it);
                break;
            }
        }
    }

    // 10. Метод find, константный (возвращающий const_iterator) и нет
    // (возвращающий iterator)

    iterator find(KeyType key) {
        size_t index = hash_func_(key) % capacity_;
        for (size_t i = 0; i < neighborhood_; ++i) {
            if (!table_[(index + i) % capacity_].empty() &&
                table_[(index + i) % capacity_].front().first == key) {
                return iterator(table_, (index + i) % capacity_,
                                table_[(index + i) % capacity_].begin());
            }
        }
        for (auto it = table_[index].begin(); it != table_[index].end(); ++it) {
            if (it->first == key) {
                return iterator(table_, index, it);
            }
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t index = hash_func_(key) % capacity_;
        for (size_t i = 0; i < neighborhood_; ++i) {
            if (!table_[(index + i) % capacity_].empty() &&
                table_[(index + i) % capacity_].front().first == key) {
                return const_iterator(table_, (index + i) % capacity_,
                                      table_[(index + i) % capacity_].begin());
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

    ValueType &operator[](KeyType key) {
        iterator it = find(key);
        if (it == end()) {
            insert({key, ValueType()});
        }
        it = find(key);
        return it->second;
    }

    // 12. Константный метод at

    const ValueType &at(KeyType key) const {
        const_iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("This key does not exist");
        }
        return it->second;
    }

    // 13. Метод clear

    void clear() {
        HashMap temp;
        swap(temp);
    }

    void swap(HashMap &other) {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(hash_func_, other.hash_func_);
        std::swap(table_, other.table_);
    }

    // 9.1 iterator

    class iterator {
    public:
        iterator() = default;

        iterator(std::vector<std::list<std::pair<const KeyType, ValueType>>> &table,
                 size_t list_index,
                 typename std::list<std::pair<const KeyType, ValueType>>::iterator inter_it)
            : table_(&table), list_index_(list_index), inter_it_(inter_it) {
        }

        std::pair<const KeyType, ValueType> &operator*() {
            return *inter_it_;
        }

        std::pair<const KeyType, ValueType> *operator->() {
            return inter_it_.operator->();
        }

        iterator &operator++() {
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

        iterator operator++(int notused) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const iterator &other) const {
            return table_ == other.table_ && list_index_ == other.list_index_ &&
                   inter_it_ == other.inter_it_;
        }

        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }

    private:
        std::vector<std::list<std::pair<const KeyType, ValueType>>> *table_;
        size_t list_index_;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator inter_it_;
    };

    iterator begin() {
        size_t index = 0;
        while (table_[index].empty() && index != capacity_ - 1) {
            ++index;
        }
        return iterator(table_, index, table_[index].begin());
    }

    iterator end() {
        return iterator(table_, capacity_ - 1, table_.back().end());
    }

    // 9.2 const_iterator

    class const_iterator {
    public:
        const_iterator() = default;

        const_iterator(
            const std::vector<std::list<std::pair<const KeyType, ValueType>>> &table,
            size_t list_index,
            typename std::list<std::pair<const KeyType, ValueType>>::const_iterator inter_it)
            : table_(&table), list_index_(list_index), inter_it_(inter_it) {
        }

        const std::pair<const KeyType, ValueType> &operator*() {
            return *inter_it_;
        }

        const std::pair<const KeyType, ValueType> *operator->() {
            return inter_it_.operator->();
        }

        const_iterator &operator++() {
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

        const_iterator operator++(int notused) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const const_iterator &other) const {
            return table_ == other.table_ && list_index_ == other.list_index_ &&
                   inter_it_ == other.inter_it_;
        }

        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }

    private:
        const std::vector<std::list<std::pair<const KeyType, ValueType>>> *table_;
        size_t list_index_;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator inter_it_;
    };

    const_iterator begin() const {
        size_t index = 0;
        while (table_[index].empty() && index != capacity_ - 1) {
            ++index;
        }
        return const_iterator(table_, index, table_[index].begin());
    }

    const_iterator end() const {
        return const_iterator(table_, capacity_ - 1, table_.back().end());
    }

protected:
    void rehash() {
        capacity_ *= 2;
        std::vector<std::list<std::pair<const KeyType, ValueType>>> temp_table(capacity_);
        std::swap(table_, temp_table);
        size_ = 0;
        for (auto &list : temp_table) {
            for (auto &elem : list) {
                insert(elem);
            }
        }
        return;
    }

private:
    std::vector<std::list<std::pair<const KeyType, ValueType>>> table_;
    Hash hash_func_;
    size_t size_, capacity_;

    static constexpr size_t neighborhood_ = 6;
    static constexpr size_t start_capacity_ = 24;
    static constexpr double load_factor_ = 0.8;
};

}  // namespace MyHashTable