#ifndef GROUP_IDENTIFICATION
#define GROUP_IDENTIFICATION

#include <xhash>

namespace qls
{

class GroupID final
{
public:
    constexpr GroupID() noexcept:
        m_group_id(0ll) {}
    constexpr explicit GroupID(long long group_id) noexcept:
        m_group_id(group_id) {}
    constexpr GroupID(const GroupID& u) noexcept:
        m_group_id(u.m_group_id) {}
    constexpr GroupID(GroupID&& u) noexcept:
        m_group_id(u.m_group_id) {}
    constexpr ~GroupID() noexcept = default;

    [[nodiscard]] constexpr long long getOriginValue() const noexcept
    {
        return m_group_id;
    }

    GroupID& operator=(const GroupID& u) noexcept
    {
        if (&u == this)
            return *this;
        m_group_id = u.m_group_id;
        return *this;
    }

    GroupID& operator=(GroupID&& u) noexcept
    {
        if (&u == this)
            return *this;
        m_group_id = u.m_group_id;
        return *this;
    }

    GroupID& operator=(long long group_id) noexcept
    {
        m_group_id = group_id;
        return *this;
    }

    friend bool operator==(const GroupID& g1, const GroupID& g2) noexcept
    {
        return g1.m_group_id == g2.m_group_id;
    }

    friend bool operator!=(const GroupID& g1, const GroupID& g2) noexcept
    {
        return g1.m_group_id != g2.m_group_id;
    }

    friend bool operator<(const GroupID& g1, const GroupID& g2) noexcept
    {
        return g1.m_group_id < g2.m_group_id;
    }

    constexpr operator long long() const noexcept
    {
        return m_group_id;
    }

private:
    long long m_group_id;
};

} // namespace qls

namespace std
{
    template<>
    struct hash<qls::GroupID>{
    public:
        std::size_t operator()(const qls::GroupID &g) const 
        {
            return hash<long long>()(g.getOriginValue());
        }
    };
    
    template<>
    struct equal_to<qls::GroupID>{
    public:
        bool operator()(const qls::GroupID &g1, const qls::GroupID &g2) const
        {
            return g1 == g2;
        }
    };
}

#endif // !GROUP_IDENTIFICATION
