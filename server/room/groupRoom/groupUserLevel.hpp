#ifndef GROUP_USER_LEVEL_HPP
#define GROUP_USER_LEVEL_HPP

#include <mutex>
#include <shared_mutex>
#include <system_error>

#include "qls_error.h"

namespace qls {

template <int MIN_Level = 1, int MAX_Level = 100>
  requires(MIN_Level <= MAX_Level)
class UserLevel final {
public:
  UserLevel(int value = MIN_Level) : m_value(MIN_Level) {
    if (MIN_Level > value || value > MAX_Level) {
      throw std::system_error(qls_errc::group_room_user_level_invalid);
    }
  }

  UserLevel(const UserLevel &user_level) : m_value(MIN_Level) {
    std::shared_lock lock(user_level.m_value_mutex);
    m_value = user_level.m_value;
  }

  UserLevel(UserLevel &&user_level) noexcept : m_value(MIN_Level) {
    std::shared_lock lock(user_level.m_value_mutex);
    m_value = user_level.m_value;
  }

  ~UserLevel() noexcept = default;

  UserLevel &operator=(const UserLevel &user_level) {
    if (&user_level == this) {
      return *this;
    }
    std::unique_lock lock_1(m_value_mutex, std::defer_lock);
    std::shared_lock lock_2(user_level.m_value_mutex, std::defer_lock);
    std::lock(lock_1, lock_2);

    m_value = user_level.m_value;
    return *this;
  }

  bool increase(int value) {
    std::unique_lock lock(m_value_mutex);
    if (MIN_Level > m_value + value || m_value + value > MAX_Level) {
      return false;
    }
    m_value += value;
    return true;
  }

  bool decrease(int value) {
    std::unique_lock lock(m_value_mutex);
    if (MIN_Level > m_value + value || m_value + value > MAX_Level) {
      return false;
    }
    m_value -= value;
    return true;
  }

  [[nodiscard]] int getValue() const {
    std::shared_lock lock(m_value_mutex);
    return m_value;
  }

private:
  mutable std::shared_mutex m_value_mutex;
  int m_value;
};

} // namespace qls

#endif // !GROUP_USER_LEVEL_HPP