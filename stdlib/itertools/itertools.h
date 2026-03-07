#ifndef SAPPHIRE_STDLIB_ITERTOOLS_H
#define SAPPHIRE_STDLIB_ITERTOOLS_H

#include <vector>
#include <functional>
#include <optional>
#include <algorithm>

namespace sapphire {
namespace stdlib {

/**
 * Itertools - Functional programming utilities for iterables
 * 
 * Provides map, filter, reduce, and other functional operations.
 */
class Itertools {
public:
    // Map - Transform each element
    template<typename T, typename R>
    static std::vector<R> map(const std::vector<T>& vec, std::function<R(T)> func) {
        std::vector<R> result;
        result.reserve(vec.size());
        for (const auto& elem : vec) {
            result.push_back(func(elem));
        }
        return result;
    }
    
    // Filter - Keep elements that match predicate
    template<typename T>
    static std::vector<T> filter(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        std::vector<T> result;
        for (const auto& elem : vec) {
            if (predicate(elem)) {
                result.push_back(elem);
            }
        }
        return result;
    }
    
    // Reduce - Combine elements into single value
    template<typename T, typename R>
    static R reduce(const std::vector<T>& vec, R initial, std::function<R(R, T)> func) {
        R result = initial;
        for (const auto& elem : vec) {
            result = func(result, elem);
        }
        return result;
    }
    
    // Take - Take first n elements
    template<typename T>
    static std::vector<T> take(const std::vector<T>& vec, size_t n) {
        size_t count = std::min(n, vec.size());
        return std::vector<T>(vec.begin(), vec.begin() + count);
    }
    
    // Drop - Skip first n elements
    template<typename T>
    static std::vector<T> drop(const std::vector<T>& vec, size_t n) {
        if (n >= vec.size()) {
            return std::vector<T>();
        }
        return std::vector<T>(vec.begin() + n, vec.end());
    }
    
    // Take while - Take elements while predicate is true
    template<typename T>
    static std::vector<T> take_while(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        std::vector<T> result;
        for (const auto& elem : vec) {
            if (!predicate(elem)) {
                break;
            }
            result.push_back(elem);
        }
        return result;
    }
    
    // Drop while - Skip elements while predicate is true
    template<typename T>
    static std::vector<T> drop_while(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        std::vector<T> result;
        bool dropping = true;
        for (const auto& elem : vec) {
            if (dropping && predicate(elem)) {
                continue;
            }
            dropping = false;
            result.push_back(elem);
        }
        return result;
    }
    
    // Zip - Combine two vectors into pairs
    template<typename T, typename U>
    static std::vector<std::pair<T, U>> zip(const std::vector<T>& vec1, const std::vector<U>& vec2) {
        std::vector<std::pair<T, U>> result;
        size_t min_size = std::min(vec1.size(), vec2.size());
        result.reserve(min_size);
        for (size_t i = 0; i < min_size; i++) {
            result.push_back({vec1[i], vec2[i]});
        }
        return result;
    }
    
    // Enumerate - Add indices to elements
    template<typename T>
    static std::vector<std::pair<size_t, T>> enumerate(const std::vector<T>& vec) {
        std::vector<std::pair<size_t, T>> result;
        result.reserve(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            result.push_back({i, vec[i]});
        }
        return result;
    }
    
    // Chunk - Split into chunks of size n
    template<typename T>
    static std::vector<std::vector<T>> chunk(const std::vector<T>& vec, size_t n) {
        std::vector<std::vector<T>> result;
        for (size_t i = 0; i < vec.size(); i += n) {
            size_t end = std::min(i + n, vec.size());
            result.push_back(std::vector<T>(vec.begin() + i, vec.begin() + end));
        }
        return result;
    }
    
    // Flatten - Flatten nested vectors
    template<typename T>
    static std::vector<T> flatten(const std::vector<std::vector<T>>& vec) {
        std::vector<T> result;
        for (const auto& inner : vec) {
            result.insert(result.end(), inner.begin(), inner.end());
        }
        return result;
    }
    
    // All - Check if all elements match predicate
    template<typename T>
    static bool all(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        for (const auto& elem : vec) {
            if (!predicate(elem)) {
                return false;
            }
        }
        return true;
    }
    
    // Any - Check if any element matches predicate
    template<typename T>
    static bool any(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        for (const auto& elem : vec) {
            if (predicate(elem)) {
                return true;
            }
        }
        return false;
    }
    
    // Find - Find first element matching predicate
    template<typename T>
    static std::optional<T> find(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        for (const auto& elem : vec) {
            if (predicate(elem)) {
                return elem;
            }
        }
        return std::nullopt;
    }
    
    // Find index - Find index of first element matching predicate
    template<typename T>
    static std::optional<size_t> find_index(const std::vector<T>& vec, std::function<bool(T)> predicate) {
        for (size_t i = 0; i < vec.size(); i++) {
            if (predicate(vec[i])) {
                return i;
            }
        }
        return std::nullopt;
    }
    
    // Partition - Split into two vectors based on predicate
    template<typename T>
    static std::pair<std::vector<T>, std::vector<T>> partition(
        const std::vector<T>& vec, 
        std::function<bool(T)> predicate
    ) {
        std::vector<T> true_vec, false_vec;
        for (const auto& elem : vec) {
            if (predicate(elem)) {
                true_vec.push_back(elem);
            } else {
                false_vec.push_back(elem);
            }
        }
        return {true_vec, false_vec};
    }
    
    // Unique - Remove duplicates (preserves order)
    template<typename T>
    static std::vector<T> unique(const std::vector<T>& vec) {
        std::vector<T> result;
        std::set<T> seen;
        for (const auto& elem : vec) {
            if (seen.find(elem) == seen.end()) {
                result.push_back(elem);
                seen.insert(elem);
            }
        }
        return result;
    }
    
    // Reverse - Reverse order
    template<typename T>
    static std::vector<T> reverse(const std::vector<T>& vec) {
        return std::vector<T>(vec.rbegin(), vec.rend());
    }
    
    // Sort - Sort elements
    template<typename T>
    static std::vector<T> sort(const std::vector<T>& vec) {
        std::vector<T> result = vec;
        std::sort(result.begin(), result.end());
        return result;
    }
    
    // Sort by - Sort using custom comparator
    template<typename T>
    static std::vector<T> sort_by(const std::vector<T>& vec, std::function<bool(T, T)> comparator) {
        std::vector<T> result = vec;
        std::sort(result.begin(), result.end(), comparator);
        return result;
    }
};

} // namespace stdlib
} // namespace sapphire

#endif // SAPPHIRE_STDLIB_COLLECTIONS_H
