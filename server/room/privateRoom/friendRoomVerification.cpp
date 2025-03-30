#include "friendRoomVerification.h"

#include <system_error>

#include "qls_error.h"

namespace qls
{

FriendRoomVerification::FriendRoomVerification(UserID user_id_1, UserID user_id_2):
    m_user_id_1(user_id_1),
    m_user_id_2(user_id_2),
    m_user_1_is_verified(false),
    m_user_2_is_verified(false)
{
}

FriendRoomVerification::FriendRoomVerification(const FriendRoomVerification& f):
    m_user_id_1(f.m_user_id_1),
    m_user_id_2(f.m_user_id_2),
    m_user_1_is_verified(static_cast<bool>(f.m_user_1_is_verified)),
    m_user_2_is_verified(static_cast<bool>(f.m_user_2_is_verified))
{
}

FriendRoomVerification::FriendRoomVerification(FriendRoomVerification&& f) noexcept:
    m_user_id_1(f.m_user_id_1),
    m_user_id_2(f.m_user_id_2),
    m_user_1_is_verified(static_cast<bool>(f.m_user_1_is_verified)),
    m_user_2_is_verified(static_cast<bool>(f.m_user_2_is_verified))
{
}

void FriendRoomVerification::setUserVerified(UserID user_id)
{
    if (user_id != m_user_id_1 && user_id != m_user_id_2)
        throw std::system_error(make_error_code(qls_errc::user_not_existed));

    if (user_id == m_user_id_1)
        m_user_1_is_verified = true;
    else if (user_id == m_user_id_2)
        m_user_2_is_verified = true;
}

bool FriendRoomVerification::getUserVerified(UserID user_id) const
{
    if (user_id != m_user_id_1 && user_id != m_user_id_2)
        throw std::system_error(make_error_code(qls_errc::user_not_existed));

    if (user_id == m_user_id_1)
        return m_user_1_is_verified;
    else
        return m_user_2_is_verified;
}

} // namespace qls
