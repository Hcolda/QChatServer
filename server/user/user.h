#ifndef USER_H
#define USER_H

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <string>

#include "groupid.hpp"
#include "userid.hpp"
#include "definition.hpp"
#include "connection.hpp"

namespace qls
{

struct Verification
{
    /**
     * @brief 验证类型:
     * @brief {Unknown:  无状态}
     * @brief {Sent:     发出的申请}
     * @brief {Received: 接收的申请}
     */
    enum VerificationType
    {
        Unknown = 0,
        Sent,
        Received
    };

    struct UserVerification
    {
        UserID user_id = UserID(0LL);
        VerificationType verification_type = Unknown;
        std::string message;
    };

    struct GroupVerification
    {
        UserID user_id = UserID(0LL);
        GroupID group_id = GroupID(0LL);
        VerificationType verification_type = Unknown;
        std::string message;
    };
};

struct UserImpl;
struct UserImplDeleter
{
    void operator()(UserImpl*);
};

/**
 * @brief Class representing a User.
 */
class User final
{
public:
    /**
     * @brief Constructor to initialize a User object.
     * @param user_id The ID of the user.
     * @param is_create Flag indicating if user is being created.
     */
    User(UserID user_id, bool is_create);

    User(const User&) = delete; // Copy constructor deleted
    User(User&&) = delete; // Move constructor deleted
    ~User();

    // Methods to get user information

    [[nodiscard]] UserID      getUserID() const;
    [[nodiscard]] std::string getUserName() const;
    [[nodiscard]] long long   getRegisteredTime() const;
    [[nodiscard]] int         getAge() const;
    [[nodiscard]] std::string getUserEmail() const;
    [[nodiscard]] std::string getUserPhone() const;
    [[nodiscard]] std::string getUserProfile() const;
    [[nodiscard]] bool        isUserPassword(std::string_view) const;

    // Methods to get user associated information

    [[nodiscard]] bool userHasFriend(UserID friend_user_id) const;
    [[nodiscard]] bool userHasGroup(GroupID group_id) const;

    [[nodiscard]] std::unordered_set<UserID> getFriendList() const;
    [[nodiscard]] std::unordered_set<GroupID> getGroupList() const;

    bool addFriend(UserID friend_user_id);
    bool acceptFriend(UserID friend_user_id);
    bool rejectFriend(UserID friend_user_id);
    bool removeFriend(UserID friend_user_id);

    /**
     * @brief Retrieves the list of friend verification entries.
     * @return unordered_map containing friend verification entries.
     */
    [[nodiscard]] std::unordered_map<UserID, Verification::UserVerification>
        getFriendVerificationList() const;

    /**
     * @brief Adds a group to the user's group list.
     * @param group_id The ID of the group to add.
     * @return true if adding group was successful, false otherwise.
     */
    bool    addGroup(GroupID group_id);
    [[nodiscard]] GroupID createGroup();
    bool    acceptGroup(GroupID group_id, UserID user_id);
    bool    rejectGroup(GroupID group_id, UserID user_id);
    bool    removeGroup(GroupID group_id);
    bool    leaveGroup(GroupID group_id);

    /**
     * @brief Retrieves the list of group verification entries.
     * @return multimap containing group verification entries.
     */
    [[nodiscard]] std::multimap<GroupID,
        Verification::GroupVerification> getGroupVerificationList() const;

    /**
     * @brief Checks if the user has a specific socket.
     * @param connection_ptr Pointer to the socket to check.
     * @return true if user has the socket, false otherwise.
     */
    [[nodiscard]] bool hasConnection(
        const std::shared_ptr<Connection<asio::ip::tcp::socket>>& connection_ptr) const;

    /**
     * @brief Modifies the type of a socket in the user's socket map.
     * @param connection_ptr Pointer to the socket to modify.
     * @param type New DeviceType associated with the socket.
     */
    void modifyConnectionType(
        const std::shared_ptr<Connection<asio::ip::tcp::socket>>& connection_ptr,
        DeviceType type);

    /**
     * @brief Notifies all sockets associated with the user.
     * @param data Data to send in the notification.
     */
    void notifyAll(std::string_view data);

    /**
     * @brief Notifies sockets of a specific DeviceType associated with the user.
     * @param type DeviceType of sockets to notify.
     * @param data Data to send in the notification.
     */
    void notifyWithType(DeviceType type, std::string_view data);

    // Methods to update user information

    void updateUserName(std::string_view);
    void updateAge(int);
    void updateUserEmail(std::string_view);
    void updateUserPhone(std::string_view);
    void updateUserProfile(std::string_view);
    void firstUpdateUserPassword(std::string_view new_password);
    void updateUserPassword(
        std::string_view old_password,
        std::string_view new_password);

    void updateFriendList(
        std::function<void(std::unordered_set<UserID>&)> callback_function);
    void updateGroupList(
        std::function<void(std::unordered_set<GroupID>&)> callback_function);

    /**
     * @brief Adds a friend verification entry.
     * @tparam T Type of Verification::UserVerification.
     * @param friend_user_id The ID of the friend.
     * @param u Verification::UserVerification to add.
     */
    void addFriendVerification(
        UserID friend_user_id,
        const Verification::UserVerification& user_verification);

    /**
     * @brief Removes a friend verification entry.
     * @param friend_user_id The ID of the friend to remove verification for.
     */
    void removeFriendVerification(UserID friend_user_id);

    /**
     * @brief Adds a group verification entry.
     * @tparam T Type of Verification::UserVerification.
     * @param group_id The ID of the group.
     * @param u Verification::UserVerification to add.
     */
    void addGroupVerification(GroupID group_id,
        const Verification::GroupVerification& group_verification);

    /**
     * @brief Removes a group verification entry.
     * @param group_id The ID of the group to remove verification for.
     * @param user_id The ID of the user to remove verification for.
     */
    void removeGroupVerification(GroupID group_id, UserID user_id);

    /**
     * @brief Adds a socket to the user's socket map.
     * @param connection_ptr Pointer to the socket to add.
     * @param type DeviceType associated with the socket.
     */
    void addConnection(
        const std::shared_ptr<Connection<asio::ip::tcp::socket>>& connection_ptr,
        DeviceType type);

    /**
     * @brief Removes a socket from the user's socket map.
     * @param connection_ptr Pointer to the socket to remove.
     */
    void removeConnection(
        const std::shared_ptr<Connection<asio::ip::tcp::socket>>& connection_ptr);

private:
    std::unique_ptr<UserImpl, UserImplDeleter> m_impl;
};

} // namespace qls

#endif // !USER_H
