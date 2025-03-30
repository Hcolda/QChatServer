#ifndef GROUP_USER_LEVEL_HPP
#define GROUP_USER_LEVEL_HPP

#include <system_error>
#include <mutex>
#include <shared_mutex>

#include "qls_error.h"

namespace qls
{

template<int MIN_Level = 1, int MAX_Level = 100>
requires(MIN_Level <= MAX_Level)
class UserLevel final
{
public:
    UserLevel(int value = MIN_Level):
        m_value(MIN_Level)
    {
        if (!(MIN_Level <= value && value <= MAX_Level))
            throw std::system_error(qls_errc::group_room_user_level_invalid);
    }

    UserLevel(const UserLevel& u):
        m_value(MIN_Level)
    {
        std::shared_lock<std::shared_mutex> lock(u.m_value_mutex);
        m_value = u.m_value;
    }

    UserLevel(UserLevel&& u) noexcept:
        m_value(MIN_Level)
    {
        std::shared_lock<std::shared_mutex> lock(u.m_value_mutex);
        m_value = u.m_value;
    }

    ~UserLevel() noexcept = default;

    UserLevel& operator=(const UserLevel& u)
    {
        if (&u == this) return *this;
        std::unique_lock<std::shared_mutex> lock_1(m_value_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock_2(u.m_value_mutex, std::defer_lock);
        std::lock(lock_1, lock_2);

        m_value = u.m_value;
        return *this;
    }

    bool increase(int value)
    {
        std::unique_lock<std::shared_mutex> lock(m_value_mutex);
        if (!(MIN_Level <= m_value + value && m_value + value <= MAX_Level))
            return false;
        m_value += value;
        return true;
    }

    bool decrease(int value)
    {
        std::unique_lock<std::shared_mutex> lock(m_value_mutex);
        if (!(MIN_Level <= m_value - value && m_value - value <= MAX_Level))
            return false;
        m_value -= value;
        return true;
    }

    int getValue() const
    {
        std::shared_lock<std::shared_mutex> lock(m_value_mutex);
        return m_value;
    }

private:
    mutable std::shared_mutex   m_value_mutex;
    int                         m_value;
};

} // namespace qls

#endif // !GROUP_USER_LEVEL_HPP