#ifndef SAPPHIRE_EXECUTOR_H
#define SAPPHIRE_EXECUTOR_H

#include "future.h"
#include <queue>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>

namespace sapphire {

// Task type - a unit of async work
using Task = std::function<void()>;

// Async executor - manages async task execution
class AsyncExecutor {
private:
    std::vector<std::thread> workers_;
    std::queue<Task> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> shutdown_;
    size_t num_workers_;
    
    // Worker thread function
    void worker_thread() {
        while (true) {
            Task task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this] {
                    return shutdown_.load() || !task_queue_.empty();
                });
                
                if (shutdown_.load() && task_queue_.empty()) {
                    return;
                }
                
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
            }
            
            if (task) {
                try {
                    task();
                } catch (const std::exception& e) {
                    // Log error but don't crash
                    // TODO: Add proper error handling
                }
            }
        }
    }
    
public:
    // Create executor with specified number of worker threads
    explicit AsyncExecutor(size_t num_workers = std::thread::hardware_concurrency())
        : shutdown_(false), num_workers_(num_workers) {
        
        if (num_workers_ == 0) {
            num_workers_ = 1;
        }
        
        // Start worker threads
        for (size_t i = 0; i < num_workers_; ++i) {
            workers_.emplace_back(&AsyncExecutor::worker_thread, this);
        }
    }
    
    ~AsyncExecutor() {
        shutdown();
    }
    
    // Spawn a new async task
    void spawn(Task task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            task_queue_.push(std::move(task));
        }
        cv_.notify_one();
    }
    
    // Spawn a task and return a future
    template<typename T>
    std::shared_ptr<Future<T>> spawn_with_future(std::function<T()> func) {
        auto promise = std::make_shared<Promise<T>>();
        auto future = promise->get_future();
        
        spawn([promise, func]() {
            try {
                T result = func();
                promise->resolve(result);
            } catch (...) {
                promise->reject(std::current_exception());
            }
        });
        
        return future;
    }
    
    // Run until a specific future completes
    template<typename T>
    T run_until_complete(std::shared_ptr<Future<T>> future) {
        // Simple implementation: just wait for the future
        // In a more sophisticated implementation, this would:
        // 1. Process tasks from the queue
        // 2. Handle nested async calls
        // 3. Implement proper event loop
        return future->get();
    }
    
    // Shutdown the executor
    void shutdown() {
        if (!shutdown_.exchange(true)) {
            cv_.notify_all();
            
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }
    }
    
    // Get number of pending tasks
    size_t pending_tasks() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queue_mutex_));
        return task_queue_.size();
    }
    
    // Check if executor is running
    bool is_running() const {
        return !shutdown_.load();
    }
};

// Global executor instance
inline AsyncExecutor& get_global_executor() {
    static AsyncExecutor executor;
    return executor;
}

} // namespace sapphire

#endif // SAPPHIRE_EXECUTOR_H
