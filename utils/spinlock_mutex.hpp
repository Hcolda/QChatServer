#ifndef SPINLOCK_MUTEX_HPP
#define SPINLOCK_MUTEX_HPP

#include <atomic>
#include <mutex>

namespace qls
{

class spinlock_mutex
{
public:
    spinlock_mutex() = default;
    ~spinlock_mutex() = default;

    spinlock_mutex(const spinlock_mutex&) = delete;
    spinlock_mutex(spinlock_mutex&&) = delete;

    spinlock_mutex& operator=(const spinlock_mutex&) = delete;
    spinlock_mutex& operator=(spinlock_mutex&&) = delete;

    void lock() noexcept
    {
        while (flag_.test_and_set(std::memory_order_acquire))
            flag_.wait(true, std::memory_order_relaxed);
    }

    bool try_lock() noexcept
    {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        flag_.clear(std::memory_order_release);
        flag_.notify_one();
    }

private:
    std::atomic_flag flag_;
};

}

#endif // !SPINLOCK_MUTEX_HPP
