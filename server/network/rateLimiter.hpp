#ifndef RATE_LIMITER_HPP
#define RATE_LIMITER_HPP

#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <mutex>
#include <unordered_map>

#include "spinlock_mutex.hpp"

namespace qls {

class RateLimiter final {
public:
  RateLimiter(double global_capacity = 500.0, double single_capacity = 5.0)
      : m_global_capacity(global_capacity), m_single_capacity(single_capacity) {
  }
  ~RateLimiter() noexcept = default;

  bool allow_connection(const asio::ip::address &addr) {
    // Present timestamp
    auto now = std::chrono::steady_clock::now();
    // Get lock to keep thread-safe
    std::unique_lock<spinlock_mutex> lock(m_token_buckets_mutex);
    // Get bucket
    auto &bucket = m_token_buckets[addr];
    // Update tokens of bucket to check
    // if the host associated with address sent too much connections in a short
    // time
    bucket.tokens =
        std::min(m_single_capacity.load(),
                 bucket.tokens + (now - bucket.last_update).count() * 1e-9 *
                                     m_single_capacity);
    bucket.last_update = now;
    bool allow = bucket.tokens-- > 0;
    // Unlock the lock to keep speedy of the process
    lock.unlock();

    if (!allow)
      return false;

    // Get global token
    double local_global_token = m_global_token.load();
    // Update global tokens of bucket
    local_global_token =
        std::min(m_global_capacity.load(),
                 local_global_token + (now - m_last_update.load()).count() *
                                          1e-9 * m_global_capacity);
    if (local_global_token <= 0)
      allow = false;
    m_global_token = local_global_token - 1;
    m_last_update.store(now);
    return allow;
  }

  void set_single_capacity(double single_capacity) {
    m_single_capacity = single_capacity;
  }

  double get_single_capacity() const { return m_single_capacity; }

  void set_global_capacity(double global_capacity) {
    m_global_capacity = global_capacity;
  }

  double get_global_capacity() const { return m_global_capacity; }

  /**
   * @brief clean the buckets out of date automatically
   */
  asio::awaitable<void> auto_clean() {
    using namespace std::chrono_literals;
    asio::steady_timer timer(co_await asio::this_coro::executor);
    while (true) {
      timer.expires_after(30s);
      co_await timer.async_wait(asio::use_awaitable);
      std::lock_guard<spinlock_mutex> lock(m_token_buckets_mutex);
      std::erase_if(m_token_buckets, [](const auto &i) {
        return std::chrono::steady_clock::now() - i.second.last_update >= 1min;
      });
    }
  }

private:
  struct TokenBucket {
    double tokens = 0.0;
    std::chrono::steady_clock::time_point last_update;
  };
  std::atomic<double> m_global_capacity;
  std::atomic<double> m_global_token;
  std::atomic<double> m_single_capacity;
  std::atomic<std::chrono::steady_clock::time_point> m_last_update;
  std::unordered_map<asio::ip::address, TokenBucket> m_token_buckets;
  mutable spinlock_mutex m_token_buckets_mutex;
};

} // namespace qls

#endif // !RATE_LIMITER_HPP
