#ifndef SAPPHIRE_FUTURE_H
#define SAPPHIRE_FUTURE_H

#include <memory>
#include <functional>
#include <vector>
#include <exception>
#include <mutex>
#include <condition_variable>

namespace sapphire {

// Forward declaration
template<typename T>
class Promise;

// Future state
enum class FutureState {
    PENDING,
    RESOLVED,
    REJECTED
};

// Future type - represents an asynchronous computation result
template<typename T>
class Future {
private:
    FutureState state_;
    T value_;
    std::exception_ptr error_;
    std::vector<std::function<void(T)>> callbacks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    friend class Promise<T>;
    
public:
    Future() : state_(FutureState::PENDING) {}
    
    // Check if the future is ready
    bool is_ready() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return state_ != FutureState::PENDING;
    }
    
    // Wait for the future to complete and get the value
    T get() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return state_ != FutureState::PENDING; });
        
        if (state_ == FutureState::REJECTED) {
            if (error_) {
                std::rethrow_exception(error_);
            }
            throw std::runtime_error("Future rejected with no error");
        }
        
        return value_;
    }
    
    // Try to get the value without blocking
    bool try_get(T& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == FutureState::RESOLVED) {
            out = value_;
            return true;
        }
        return false;
    }
    
    // Register a callback to be called when the future completes
    void then(std::function<void(T)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == FutureState::RESOLVED) {
            // Already resolved, call immediately
            callback(value_);
        } else if (state_ == FutureState::PENDING) {
            // Still pending, register callback
            callbacks_.push_back(callback);
        }
        // If rejected, don't call callback
    }
    
    // Get the current state
    FutureState state() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return state_;
    }
    
private:
    // Called by Promise to resolve the future
    void resolve(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != FutureState::PENDING) {
            return; // Already resolved or rejected
        }
        
        state_ = FutureState::RESOLVED;
        value_ = value;
        
        // Call all registered callbacks
        for (auto& callback : callbacks_) {
            callback(value_);
        }
        callbacks_.clear();
        
        cv_.notify_all();
    }
    
    // Called by Promise to reject the future
    void reject(std::exception_ptr error) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != FutureState::PENDING) {
            return; // Already resolved or rejected
        }
        
        state_ = FutureState::REJECTED;
        error_ = error;
        callbacks_.clear();
        
        cv_.notify_all();
    }
};

// Promise type - used to resolve a future
template<typename T>
class Promise {
private:
    std::shared_ptr<Future<T>> future_;
    
public:
    Promise() : future_(std::make_shared<Future<T>>()) {}
    
    // Get the associated future
    std::shared_ptr<Future<T>> get_future() {
        return future_;
    }
    
    // Resolve the promise with a value
    void resolve(T value) {
        if (future_) {
            future_->resolve(value);
        }
    }
    
    // Reject the promise with an error
    void reject(std::exception_ptr error) {
        if (future_) {
            future_->reject(error);
        }
    }
    
    // Reject with a runtime error
    void reject(const std::string& message) {
        try {
            throw std::runtime_error(message);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

} // namespace sapphire

#endif // SAPPHIRE_FUTURE_H
