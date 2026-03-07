#ifndef SAPPHIRE_STDLIB_COLLECTIONS_H
#define SAPPHIRE_STDLIB_COLLECTIONS_H

#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <optional>

namespace sapphire {
namespace stdlib {

/**
 * Set - Unordered collection of unique elements
 * 
 * Provides O(log n) insertion, deletion, and lookup.
 */
template<typename T>
class Set {
private:
    std::set<T> data_;
    
public:
    Set() = default;
    
    // Add element (returns true if inserted, false if already exists)
    bool insert(const T& value) {
        return data_.insert(value).second;
    }
    
    // Remove element (returns true if removed, false if not found)
    bool remove(const T& value) {
        return data_.erase(value) > 0;
    }
    
    // Check if element exists
    bool contains(const T& value) const {
        return data_.find(value) != data_.end();
    }
    
    // Size
    size_t size() const {
        return data_.size();
    }
    
    bool empty() const {
        return data_.empty();
    }
    
    // Clear all elements
    void clear() {
        data_.clear();
    }
    
    // Convert to vector
    std::vector<T> to_vector() const {
        return std::vector<T>(data_.begin(), data_.end());
    }
    
    // Set operations
    Set<T> union_with(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : data_) {
            result.insert(elem);
        }
        for (const auto& elem : other.data_) {
            result.insert(elem);
        }
        return result;
    }
    
    Set<T> intersection(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : data_) {
            if (other.contains(elem)) {
                result.insert(elem);
            }
        }
        return result;
    }
    
    Set<T> difference(const Set<T>& other) const {
        Set<T> result;
        for (const auto& elem : data_) {
            if (!other.contains(elem)) {
                result.insert(elem);
            }
        }
        return result;
    }
};

/**
 * Queue - FIFO queue
 */
template<typename T>
class Queue {
private:
    std::queue<T> data_;
    
public:
    Queue() = default;
    
    // Add to back
    void enqueue(const T& value) {
        data_.push(value);
    }
    
    // Remove from front (returns std::optional)
    std::optional<T> dequeue() {
        if (data_.empty()) {
            return std::nullopt;
        }
        T value = data_.front();
        data_.pop();
        return value;
    }
    
    // Peek at front without removing
    std::optional<T> peek() const {
        if (data_.empty()) {
            return std::nullopt;
        }
        return data_.front();
    }
    
    size_t size() const {
        return data_.size();
    }
    
    bool empty() const {
        return data_.empty();
    }
    
    void clear() {
        while (!data_.empty()) {
            data_.pop();
        }
    }
};

/**
 * Stack - LIFO stack
 */
template<typename T>
class Stack {
private:
    std::stack<T> data_;
    
public:
    Stack() = default;
    
    // Push to top
    void push(const T& value) {
        data_.push(value);
    }
    
    // Pop from top (returns std::optional)
    std::optional<T> pop() {
        if (data_.empty()) {
            return std::nullopt;
        }
        T value = data_.top();
        data_.pop();
        return value;
    }
    
    // Peek at top without removing
    std::optional<T> peek() const {
        if (data_.empty()) {
            return std::nullopt;
        }
        return data_.top();
    }
    
    size_t size() const {
        return data_.size();
    }
    
    bool empty() const {
        return data_.empty();
    }
    
    void clear() {
        while (!data_.empty()) {
            data_.pop();
        }
    }
};

/**
 * Deque - Double-ended queue
 */
template<typename T>
class Deque {
private:
    std::deque<T> data_;
    
public:
    Deque() = default;
    
    // Add to front
    void push_front(const T& value) {
        data_.push_front(value);
    }
    
    // Add to back
    void push_back(const T& value) {
        data_.push_back(value);
    }
    
    // Remove from front
    std::optional<T> pop_front() {
        if (data_.empty()) {
            return std::nullopt;
        }
        T value = data_.front();
        data_.pop_front();
        return value;
    }
    
    // Remove from back
    std::optional<T> pop_back() {
        if (data_.empty()) {
            return std::nullopt;
        }
        T value = data_.back();
        data_.pop_back();
        return value;
    }
    
    size_t size() const {
        return data_.size();
    }
    
    bool empty() const {
        return data_.empty();
    }
    
    void clear() {
        data_.clear();
    }
};

} // namespace stdlib
} // namespace sapphire

#endif // SAPPHIRE_STDLIB_COLLECTIONS_H
