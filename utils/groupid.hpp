#ifndef GROUP_IDENTIFICATION
#define GROUP_IDENTIFICATION

#include <functional>

namespace qls {

class GroupID final {
public:
  constexpr GroupID() noexcept : m_group_id(0LL) {}
  constexpr explicit GroupID(long long group_id) noexcept
      : m_group_id(group_id) {}
  constexpr GroupID(const GroupID &group_id) noexcept
      : m_group_id(group_id.m_group_id) {}
  constexpr GroupID(GroupID &&group_id) noexcept
      : m_group_id(group_id.m_group_id) {}
  constexpr ~GroupID() noexcept = default;

  [[nodiscard]] constexpr long long getOriginValue() const noexcept {
    return m_group_id;
  }

  GroupID &operator=(const GroupID &group_id) noexcept {
    if (&group_id == this) {
      return *this;
    }
    m_group_id = group_id.m_group_id;
    return *this;
  }

  GroupID &operator=(GroupID &&group_id) noexcept {
    if (&group_id == this) {
      return *this;
    }
    m_group_id = group_id.m_group_id;
    return *this;
  }

  GroupID &operator=(long long group_id) noexcept {
    m_group_id = group_id;
    return *this;
  }

  friend bool operator==(const GroupID &group_id1,
                         const GroupID &group_id2) noexcept {
    return group_id1.m_group_id == group_id2.m_group_id;
  }

  friend bool operator!=(const GroupID &group_id1,
                         const GroupID &group_id2) noexcept {
    return group_id1.m_group_id != group_id2.m_group_id;
  }

  friend bool operator<(const GroupID &group_id1,
                        const GroupID &group_id2) noexcept {
    return group_id1.m_group_id < group_id2.m_group_id;
  }

  constexpr operator long long() const noexcept { return m_group_id; }

private:
  long long m_group_id;
};

} // namespace qls

namespace std {
template <> struct hash<qls::GroupID> {
public:
  std::size_t operator()(const qls::GroupID &group_id) const {
    return hash<long long>()(group_id.getOriginValue());
  }
};

template <> struct equal_to<qls::GroupID> {
public:
  bool operator()(const qls::GroupID &group_id1,
                  const qls::GroupID &group_id2) const {
    return group_id1 == group_id2;
  }
};
} // namespace std

#endif // !GROUP_IDENTIFICATION
