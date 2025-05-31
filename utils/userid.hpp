#ifndef USER_IDENTIFICATION
#define USER_IDENTIFICATION

#include <functional>

namespace qls {

class UserID final {
public:
  constexpr UserID() noexcept : m_user_id(0LL) {}
  constexpr explicit UserID(long long user_id) noexcept : m_user_id(user_id) {}
  constexpr UserID(const UserID &user_id) noexcept
      : m_user_id(user_id.m_user_id) {}
  constexpr UserID(UserID &&user_id) noexcept : m_user_id(user_id.m_user_id) {}
  constexpr ~UserID() noexcept = default;

  [[nodiscard]] constexpr long long getOriginValue() const noexcept {
    return m_user_id;
  }

  UserID &operator=(const UserID &user_id) noexcept {
    if (&user_id == this) {
      return *this;
    }
    m_user_id = user_id.m_user_id;
    return *this;
  }

  UserID &operator=(UserID &&user_id) noexcept {
    if (&user_id == this) {
      return *this;
    }
    m_user_id = user_id.m_user_id;
    return *this;
  }

  UserID &operator=(long long user_id) noexcept {
    m_user_id = user_id;
    return *this;
  }

  friend bool operator==(const UserID &user_id1,
                         const UserID &user_id2) noexcept {
    return user_id1.m_user_id == user_id2.m_user_id;
  }

  friend bool operator!=(const UserID &user_id1,
                         const UserID &user_id2) noexcept {
    return user_id1.m_user_id != user_id2.m_user_id;
  }

  friend bool operator<(const UserID &user_id1,
                        const UserID &user_id2) noexcept {
    return user_id1.m_user_id < user_id2.m_user_id;
  }

  constexpr operator long long() const noexcept { return m_user_id; }

private:
  long long m_user_id;
};

} // namespace qls

namespace std {
template <> struct hash<qls::UserID> {
public:
  std::size_t operator()(const qls::UserID &user_id) const {
    return hash<long long>()(user_id.getOriginValue());
  }
};

template <> struct equal_to<qls::UserID> {
public:
  bool operator()(const qls::UserID &user_id1,
                  const qls::UserID &user_id2) const {
    return user_id1 == user_id2;
  }
};
} // namespace std

#endif // !USER_IDENTIFICATION
