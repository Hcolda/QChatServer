#ifndef VERIFICATION_MANAGER_H
#define VERIFICATION_MANAGER_H

#include <unordered_map>
#include <shared_mutex>
#include <Json.h>

#include "userid.hpp"
#include "groupid.hpp"
#include "friendRoomVerification.h"
#include "groupRoomVerification.h"
#include "definition.hpp"

namespace qls
{
    
/**
 * @class VerificationManager
 * @brief Manages verifications for friend and group room requests.
 */
class VerificationManager final
{
public:
    VerificationManager() = default;
    VerificationManager(const VerificationManager&) = delete;
    VerificationManager(VerificationManager&&) = delete;
    ~VerificationManager() = default;

    VerificationManager& operator=(const VerificationManager&) = delete;
    VerificationManager& operator=(VerificationManager&&) = delete;

    /**
     * @brief Initializes the verification manager.
     */
    void init();

    /**
     * @brief Adds a friend room verification request between two users.
     * @param user_id_1 ID of the first user.
     * @param user_id_2 ID of the second user.
     */
    void addFriendRoomVerification(UserID user_id_1, UserID user_id_2);

    /**
     * @brief Checks if there is a friend room verification request between two users.
     * @param user_id_1 ID of the first user.
     * @param user_id_2 ID of the second user.
     * @return True if there is a verification request, false otherwise.
     */
    [[nodiscard]] bool hasFriendRoomVerification(UserID user_id_1, UserID user_id_2) const;

    /**
     * @brief Sets a friend verification status.
     * @param user_id_1 ID of the first user.
     * @param user_id_2 ID of the second user.
     * @param user_id ID of the user setting the verification.
     * @return True if verification status was successfully set, false otherwise.
     */
    [[nodiscard]] bool setFriendVerified(UserID user_id_1, UserID user_id_2, UserID user_id);

    /**
     * @brief Removes a friend room verification request.
     * @param user_id_1 ID of the first user.
     * @param user_id_2 ID of the second user.
     */
    void removeFriendRoomVerification(UserID user_id_1, UserID user_id_2);

    /**
     * @brief Adds a group room verification request.
     * @param group_id ID of the group.
     * @param user_id ID of the user requesting to join the group.
     */
    void addGroupRoomVerification(GroupID group_id, UserID user_id);

    /**
     * @brief Checks if there is a group room verification request.
     * @param group_id ID of the group.
     * @param user_id ID of the user requesting to join the group.
     * @return True if there is a verification request, false otherwise.
     */
    [[nodiscard]] bool hasGroupRoomVerification(GroupID group_id, UserID user_id) const;

    /**
     * @brief Sets the group verification status for a group.
     * @param group_id ID of the group.
     * @param user_id ID of the user setting the verification.
     * @return True if verification status was successfully set, false otherwise.
     */
    [[nodiscard]] bool setGroupRoomGroupVerified(GroupID group_id, UserID user_id);

    /**
     * @brief Sets the user verification status for a group.
     * @param group_id ID of the group.
     * @param user_id ID of the user setting the verification.
     * @return True if verification status was successfully set, false otherwise.
     */
    [[nodiscard]] bool setGroupRoomUserVerified(GroupID group_id, UserID user_id);

    /**
     * @brief Removes a group room verification request.
     * @param group_id ID of the group.
     * @param user_id ID of the user requesting to join the group.
     */
    void removeGroupRoomVerification(GroupID group_id, UserID user_id);

private:
    /**
     * @brief Map of friend room verification requests.
     */
    std::unordered_map<PrivateRoomIDStruct,
        qls::FriendRoomVerification,
        PrivateRoomIDStructHasher>          m_FriendRoomVerification_map;
    mutable std::shared_mutex               m_FriendRoomVerification_map_mutex; ///< Mutex for friend room verification map.

    /**
     * @brief Map of group room verification requests.
     */
    std::unordered_map<GroupVerificationStruct,
        qls::GroupRoomVerification,
        GroupVerificationStructHasher>      m_GroupVerification_map;
    mutable std::shared_mutex               m_GroupVerification_map_mutex; ///< Mutex for group room verification map.
};

} // namespace qls

#endif // !VERIFICATION_MANAGER_H
