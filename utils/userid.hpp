#ifndef USER_IDENTIFICATION
#define USER_IDENTIFICATION

#include <functional>

namespace qls {

class UserID final {
public:
  constexpr UserID() noexcept : m_user_id(0ll) {}
  constexpr explicit UserID(long long user_id) noexcept : m_user_id(user_id) {}
  constexpr UserID(const UserID &u) noexcept : m_user_id(u.m_user_id) {}
  constexpr UserID(UserID &&u) noexcept : m_user_id(u.m_user_id) {}
  constexpr ~UserID() noexcept = default;

  [[nodiscard]] constexpr long long getOriginValue() const noexcept {
    return m_user_id;
  }

  UserID &operator=(const UserID &u) noexcept {
    if (&u == this)
      return *this;
    m_user_id = u.m_user_id;
    return *this;
  }

  UserID &operator=(UserID &&u) noexcept {
    if (&u == this)
      return *this;
    m_user_id = u.m_user_id;
    return *this;
  }

  UserID &operator=(long long user_id) noexcept {
    m_user_id = user_id;
    return *this;
  }

  friend bool operator==(const UserID &u1, const UserID &u2) noexcept {
    return u1.m_user_id == u2.m_user_id;
  }

  friend bool operator!=(const UserID &u1, const UserID &u2) noexcept {
    return u1.m_user_id != u2.m_user_id;
  }

  friend bool operator<(const UserID &u1, const UserID &u2) noexcept {
    return u1.m_user_id < u2.m_user_id;
  }

  constexpr operator long long() const noexcept { return m_user_id; }

private:
  long long m_user_id;
};

} // namespace qls

namespace std {
template <> struct hash<qls::UserID> {
public:
  std::size_t operator()(const qls::UserID &u) const {
    return hash<long long>()(u.getOriginValue());
  }
};

template <> struct equal_to<qls::UserID> {
public:
  bool operator()(const qls::UserID &u1, const qls::UserID &u2) const {
    return u1 == u2;
  }
};
} // namespace std

#endif // !USER_IDENTIFICATION
