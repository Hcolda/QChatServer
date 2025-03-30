#ifndef FRIEND_ROOM_VERIFICATION_H
#define FRIEND_ROOM_VERIFICATION_H

#include <atomic>

#include "userid.hpp"

namespace qls
{

/**
 * @brief Class representing verification status for two friends.
 */
class FriendRoomVerification final
{
public:
    /**
     * @brief Constructor to initialize FriendRoomVerification object.
     * @param user_id_1 ID of the first user.
     * @param user_id_2 ID of the second user.
     */
    FriendRoomVerification(UserID user_id_1, UserID user_id_2);

    /**
     * @brief Copy constructor.
     * @param frv Another FriendRoomVerification object to copy from.
     */
    FriendRoomVerification(const FriendRoomVerification&);

    /**
     * @brief Move constructor.
     * @param frv Another FriendRoomVerification object to move from.
     */
    FriendRoomVerification(FriendRoomVerification&&) noexcept;

    /**
     * @brief Destructor (defaulted).
     */
    ~FriendRoomVerification() noexcept = default;

    /**
     * @brief Sets the verification status of a user.
     * @param user_id ID of the user.
     */
    void setUserVerified(UserID user_id);

    /**
     * @brief Gets the verification status of a user.
     * @param user_id ID of the user.
     * @return true if user is verified, false otherwise.
     */
    bool getUserVerified(UserID user_id) const;

private:
    const UserID        m_user_id_1; ///< ID of the first user
    const UserID        m_user_id_2; ///< ID of the second user
    std::atomic<bool>   m_user_1_is_verified; ///< Atomic flag for verification status of the first user
    std::atomic<bool>   m_user_2_is_verified; ///< Atomic flag for verification status of the second user
};

} // namespace qls

#endif // !FRIEND_ROOM_VERIFICATION_H
