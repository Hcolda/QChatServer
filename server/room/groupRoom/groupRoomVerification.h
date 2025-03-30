#ifndef GROUP_ROOM_VERIFICATION_H
#define GROUP_ROOM_VERIFICATION_H

#include <atomic>

#include "groupid.hpp"
#include "userid.hpp"

namespace qls
{

/**
 * @brief Class representing verification status for a user within a group chat room.
 */
class GroupRoomVerification final
{
public:
    /**
     * @brief Constructor to initialize GroupRoomVerification object.
     * @param group_id ID of the group.
     * @param user_id ID of the user.
     */
    GroupRoomVerification(GroupID group_id, UserID user_id);

    /**
     * @brief Copy constructor.
     * @param grv Another GroupRoomVerification object to copy from.
     */
    GroupRoomVerification(const GroupRoomVerification&);

    /**
     * @brief Move constructor.
     * @param grv Another GroupRoomVerification object to move from.
     */
    GroupRoomVerification(GroupRoomVerification&&) noexcept;

    /**
     * @brief Destructor (defaulted).
     */
    ~GroupRoomVerification() noexcept = default;

    /**
     * @brief Sets the verification status of the group chat room.
     */
    void setGroupVerified();

    /**
     * @brief Gets the verification status of the group chat room.
     * @return true if group is verified, false otherwise.
     */
    bool getGroupVerified() const;

    /**
     * @brief Sets the verification status of the user.
     */
    void setUserVerified();

    /**
     * @brief Gets the verification status of the user.
     * @return true if user is verified, false otherwise.
     */
    bool getUserVerified() const;

private:
    const GroupID       m_group_id; ///< ID of the group
    const UserID        m_user_id;  ///< ID of the user
    std::atomic<bool>   m_group_is_verified; ///< Atomic flag for group verification status
    std::atomic<bool>   m_user_is_verified;  ///< Atomic flag for user verification status
};

} // namespace qls

#endif // !GROUP_ROOM_VERIFICATION_H
