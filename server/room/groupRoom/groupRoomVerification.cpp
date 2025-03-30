#include "groupRoomVerification.h"

namespace qls
{

GroupRoomVerification::GroupRoomVerification(GroupID group_id, UserID user_id) :
    m_group_id(group_id),
    m_user_id(user_id),
    m_group_is_verified(false),
    m_user_is_verified(false)
{
}

GroupRoomVerification::GroupRoomVerification(const GroupRoomVerification& g) :
    m_group_id(g.m_group_id),
    m_user_id(g.m_user_id),
    m_group_is_verified(static_cast<bool>(g.m_group_is_verified)),
    m_user_is_verified(static_cast<bool>(g.m_user_is_verified))
{
}

GroupRoomVerification::GroupRoomVerification(GroupRoomVerification&& g) noexcept :
    m_group_id(g.m_group_id),
    m_user_id(g.m_user_id),
    m_group_is_verified(static_cast<bool>(g.m_group_is_verified)),
    m_user_is_verified(static_cast<bool>(g.m_user_is_verified))
{
}

void GroupRoomVerification::setGroupVerified()
{
    m_group_is_verified = true;
}

bool GroupRoomVerification::getGroupVerified() const
{
    return m_group_is_verified;
}

void GroupRoomVerification::setUserVerified()
{
    m_user_is_verified = true;
}

bool GroupRoomVerification::getUserVerified() const
{
    return m_user_is_verified;
}

} // namespace qls
