#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace iot::utils {

/**
 * @brief A thread-safe queue for inter-thread communication.
 * Inspired by Go channels. Supports blocking receive with timeout.
 */
template <typename T>
class ThreadQueue {
public:
    ThreadQueue() = default;
    ~ThreadQueue() = default;

    // Disallow copy/move
    ThreadQueue(const ThreadQueue&) = delete;
    ThreadQueue& operator=(const ThreadQueue&) = delete;

    /**
     * @brief Send an item into the channel and notify waiting receivers.
     */
    void send(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_.notify_one();
    }

    /**
     * @brief Receive an item from the channel. Blocks until available or timeout.
     * @param value Out parameter to store the received item.
     * @param timeoutMs Timeout in milliseconds to wait.
     * @return true if an item was received, false on timeout.
     */
    bool receive(T& value, int timeoutMs = 100) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !queue_.empty(); })) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /**
     * @brief Get current size of the queue buffer.
     */
    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief Check if the queue buffer is empty.
     */
    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Clear all items from the queue buffer.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<T> emptyQueue;
        std::swap(queue_, emptyQueue);
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace iot::utils
