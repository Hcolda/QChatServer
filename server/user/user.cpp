#include "user.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <random>
#include <asio.hpp>
#include <memory_resource>
#include <ranges>

#include "Json.h"
#include "manager.h"
#include "logger.hpp"
#include "qls_error.h"
#include "dataPackage.h"
#include "userid.hpp"
#include "groupid.hpp"
#include "ossl_proxy.hpp"
#include "md_proxy.hpp"
#include "md_ctx_proxy.hpp"

extern Log::Logger serverLogger;
extern qls::Manager serverManager;

namespace qls
{

static std::pmr::synchronized_pool_resource local_user_sync_pool;
struct UserImpl
{
    UserID      user_id; ///< User ID
    std::string user_name; ///< User name
    long long   registered_time; ///< Time when user registered
    int         age; ///< User's age
    std::string email; ///< User's email
    std::string phone; ///< User's phone number
    std::string profile; ///< User profile

    std::string password; ///< User's hashed password
    std::string salt; ///< Salt used in password hashing

    std::shared_mutex   m_data_mutex; ///< Mutex for thread-safe access to user data

    std::unordered_set<UserID>      m_user_friend_set; ///< User's friend list
    std::shared_mutex               m_user_friend_set_mutex; ///< Mutex for thread-safe access to friend list

    std::unordered_map<UserID, Verification::UserVerification>
                                    m_user_friend_verification_map; ///< User's friend verification map
    std::shared_mutex               m_user_friend_verification_map_mutex; ///< Mutex for thread-safe access to friend verification map

    std::unordered_set<GroupID>     m_user_group_set; ///< User's group list
    std::shared_mutex               m_user_group_set_mutex; ///< Mutex for thread-safe access to group list

    std::multimap<GroupID, Verification::GroupVerification>
                                    m_user_group_verification_map; ///< User's group verification map
    std::shared_mutex               m_user_group_verification_map_mutex; ///< Mutex for thread-safe access to group verification map

    std::unordered_map<std::shared_ptr<Connection<asio::ip::tcp::socket>>, DeviceType>
                                    m_connection_map; ///< Map of sockets associated with the user
    std::shared_mutex               m_connection_map_mutex; ///< Mutex for thread-safe access to socket map

    inline static ossl_proxy        m_ossl_proxy = {};
    inline static md_proxy          m_md_proxy = {m_ossl_proxy, "SHA3-512"};

    bool removeFriend(UserID friend_user_id)
    {
        std::unique_lock ul(m_user_friend_set_mutex);
        auto iter = m_user_friend_set.find(friend_user_id);
        if (iter == m_user_friend_set.cend())
            return false;

        m_user_friend_set.erase(iter);
        return true;
    }
};

/**
 * @brief Send json message to user
 * @param user_id The id of user
 * @param json json type message
*/
template<class T>
    requires requires (T json_value) { qjson::JObject(json_value); }
static inline void sendJsonToUser(qls::UserID user_id, T&& json)
{
    auto pack = DataPackage::makePackage(
        qjson::JObject(std::forward<T>(json)).to_string());
    pack->type = DataPackage::Text;
    serverManager.getUser(user_id)->notifyAll(pack->packageToString());
}

template<class T, std::input_iterator It, std::sentinel_for<It> S>
    requires requires (T json_value) { qjson::JObject(json_value); }
static inline void sendJsonToUser(It begin, S end, T&& json)
{
    auto pack = DataPackage::makePackage(
        qjson::JObject(std::forward<T>(json)).to_string());
    pack->type = DataPackage::Text;
    std::ranges::for_each(begin, end,
        [pack = std::move(pack)](const qls::UserID& user_id) {
        serverManager.getUser(user_id)->notifyAll(pack->packageToString());
        }
    );
}

User::User(UserID user_id, bool is_create):
    // allocate and construct the pointer of implement
    m_impl(std::pmr::polymorphic_allocator<>(
        &local_user_sync_pool).new_object<UserImpl>())
{
    m_impl->user_id = user_id;
    m_impl->age = 0;
    m_impl->registered_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now()).time_since_epoch().count();

    if (is_create) {
        // Update database
    } else {
        // Read the data from database
    }
}

User::~User() = default;

UserID User::getUserID() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->user_id;
}

std::string User::getUserName() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->user_name;
}

long long User::getRegisteredTime() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->registered_time;
}

int User::getAge() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->age;
}

std::string User::getUserEmail() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->email;
}
std::string User::getUserPhone() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->phone;
}

std::string User::getUserProfile() const
{
    std::shared_lock lock(m_impl->m_data_mutex);
    return m_impl->profile;
}

bool User::isUserPassword(std::string_view password) const
{
    md_ctx_proxy sha512_proxy(m_impl->m_md_proxy);
    std::shared_lock lock(m_impl->m_data_mutex);
    std::string localPassword = sha512_proxy(password, m_impl->salt);

    return localPassword == m_impl->password;
}

void User::updateUserName(std::string_view user_name)
{
    std::unique_lock lg(m_impl->m_data_mutex);
    m_impl->user_name = user_name;
}

void User::updateAge(int age)
{
    std::unique_lock lg(m_impl->m_data_mutex);
    m_impl->age = age;
}

void User::updateUserEmail(std::string_view email)
{
    std::unique_lock lg(m_impl->m_data_mutex);
    m_impl->email = email;
}

void User::updateUserPhone(std::string_view phone)
{
    std::unique_lock lg(m_impl->m_data_mutex);
    m_impl->phone = phone;
}

void User::updateUserProfile(std::string_view profile)
{
    std::unique_lock lg(m_impl->m_data_mutex);
    m_impl->profile = profile;
}

void User::firstUpdateUserPassword(std::string_view new_password)
{
    if (!m_impl->password.empty())
        throw std::system_error(qls_errc::password_already_set);

    md_ctx_proxy sha512_proxy(m_impl->m_md_proxy);
    static std::mt19937_64 mt(std::random_device{}());

    // Generate salt and hash of password
    std::string localSalt = std::to_string(mt());
    std::string localPassword = sha512_proxy(new_password, localSalt);

    {
        std::unique_lock lock(m_impl->m_data_mutex);
        m_impl->password = localPassword;
        m_impl->salt = localSalt;
    }
    {
        // Update database
    }
}

void User::updateUserPassword(
    std::string_view old_password,
    std::string_view new_password)
{
    if (!isUserPassword(old_password))
        throw std::system_error(qls_errc::password_mismatched,
            "wrong old password");

    md_ctx_proxy sha512_proxy(m_impl->m_md_proxy);
    static std::mt19937_64 mt(std::random_device{}());

    // Generate salt and hash of password
    std::string localSalt = std::to_string(mt());
    std::string localPassword = sha512_proxy(new_password, localSalt);

    {
        std::unique_lock lock(m_impl->m_data_mutex);
        m_impl->password = localPassword;
        m_impl->salt = localSalt;
    }
    {
        // Update database
    }
}

bool User::userHasFriend(UserID friend_user_id) const
{
    std::shared_lock
        lock(m_impl->m_user_friend_set_mutex);
    return m_impl->m_user_friend_set.find(friend_user_id) !=
        m_impl->m_user_friend_set.cend();
}

bool User::userHasGroup(GroupID group_id) const
{
    std::shared_lock
        lock(m_impl->m_user_group_set_mutex);
    return m_impl->m_user_group_set.find(group_id) !=
        m_impl->m_user_group_set.cend();
}

std::unordered_set<UserID> User::getFriendList() const
{
    std::shared_lock lock(m_impl->m_user_friend_set_mutex);
    return m_impl->m_user_friend_set;
}

std::unordered_set<GroupID> User::getGroupList() const
{
    std::shared_lock lock(m_impl->m_user_group_set_mutex);
    return m_impl->m_user_group_set;
}

bool User::addFriend(UserID friend_user_id)
{
    UserID self_id = this->getUserID();
    if (self_id == friend_user_id || !serverManager.hasUser(friend_user_id))
        return false;

    // check if they are friends
    if (!serverManager.hasPrivateRoom(self_id, friend_user_id))
        return false;

    auto& ver = serverManager.getServerVerificationManager();
    if (ver.hasFriendRoomVerification(self_id, friend_user_id))
        return false;

    ver.applyFriendRoomVerification(self_id, friend_user_id);

    // notify the other successfully adding a friend
    qjson::JObject json(qjson::JValueType::JDict);
    json["userid"] = self_id.getOriginValue();
    json["type"] = "added_friend_verfication";
    json["message"] = "";
    sendJsonToUser(friend_user_id, std::move(json));
    return true;
}

bool User::acceptFriend(UserID friend_user_id)
{
    UserID self_id = this->getUserID();
    if (self_id == friend_user_id ||
        !serverManager.hasUser(friend_user_id) ||
        !serverManager.getServerVerificationManager()
            .hasFriendRoomVerification(self_id, friend_user_id))
        return false;

    serverManager.getServerVerificationManager().acceptFriendVerification(friend_user_id, self_id);

    // notify the other successfully adding a friend
    qjson::JObject json(qjson::JValueType::JDict);
    json["userid"] = self_id.getOriginValue();
    json["type"] = "added_friend";
    sendJsonToUser(friend_user_id, std::move(json));
    return true;
}

bool User::rejectFriend(UserID friend_user_id)
{
    UserID self_id = this->getUserID();
    if (self_id == friend_user_id ||
        !serverManager.hasUser(friend_user_id) ||
        !serverManager.getServerVerificationManager()
            .hasFriendRoomVerification(self_id, friend_user_id))
        return false;

    try {
        serverManager.getServerVerificationManager()
            .removeFriendRoomVerification(self_id, friend_user_id);
    } catch(...) {
        return false;
    }

    // notify them to remove the friend verification
    // (someone reject to add a friend)
    qjson::JObject json(qjson::JValueType::JDict);
    json["userid"] = self_id.getOriginValue();
    json["type"] = "rejected_to_add_friend";
    sendJsonToUser(friend_user_id, std::move(json));
    return true;
}

bool User::removeFriend(UserID friend_user_id)
{
    UserID self_id = this->getUserID();
    if (self_id == friend_user_id || !serverManager.hasUser(friend_user_id))
        return false;

    if (!this->userHasFriend(friend_user_id))
        return false;

    m_impl->removeFriend(friend_user_id);
    serverManager.getUser(friend_user_id)->m_impl->removeFriend(friend_user_id);
    
    // notify them to remove the friend verification
    // (someone reject to add a friend)
    qjson::JObject json(qjson::JValueType::JDict);
    json["userid"] = self_id.getOriginValue();
    json["type"] = "removed_friend";
    sendJsonToUser(friend_user_id, std::move(json));
    return true;
}

void User::updateFriendList(
    std::function<void(std::unordered_set<UserID>&)> callback_function)
{
    if (!callback_function)
        throw std::system_error(make_error_code(qls::qls_errc::null_pointer));

    std::unique_lock lock(m_impl->m_user_friend_set_mutex);
    callback_function(m_impl->m_user_friend_set);
}

void User::updateGroupList(
    std::function<void(std::unordered_set<GroupID>&)> callback_function)
{
    if (!callback_function)
        throw std::system_error(make_error_code(qls::qls_errc::null_pointer));

    std::unique_lock
        lock(m_impl->m_user_group_set_mutex);
    callback_function(m_impl->m_user_group_set);
}

void User::addFriendVerification(
    UserID friend_user_id,
    const Verification::UserVerification& user_verification)
{
    std::unique_lock
        lock(m_impl->m_user_friend_verification_map_mutex);
    m_impl->m_user_friend_verification_map.emplace(friend_user_id, user_verification);
}

void User::addGroupVerification(
    GroupID group_id,
    const Verification::GroupVerification& group_verification)
{
    std::unique_lock
        lock(m_impl->m_user_group_verification_map_mutex);
    m_impl->m_user_group_verification_map.insert({ group_id, group_verification });
}

void User::removeFriendVerification(UserID friend_user_id)
{
    std::unique_lock
        lock(m_impl->m_user_friend_verification_map_mutex);
    auto itor = m_impl->m_user_friend_verification_map.find(friend_user_id);
    if (itor == m_impl->m_user_friend_verification_map.cend())
        return;
    m_impl->m_user_friend_verification_map.erase(itor);
}

std::unordered_map<UserID, Verification::UserVerification>
    User::getFriendVerificationList() const
{
    std::shared_lock
        lock(m_impl->m_user_friend_verification_map_mutex);
    return m_impl->m_user_friend_verification_map;
}

bool User::addGroup(GroupID group_id)
{
    UserID self_id = this->getUserID();
    if (!serverManager.hasGroupRoom(group_id) ||
        serverManager.getGroupRoom(group_id)->hasMember(self_id))
            return false;

    auto& ver = serverManager.getServerVerificationManager();
    if (ver.hasGroupRoomVerification(self_id, group_id))
        return false;

    ver.applyGroupRoomVerification(self_id, group_id);

    UserID adminID = serverManager.getGroupRoom(group_id)->getAdministrator();
    qjson::JObject json(qjson::JValueType::JDict);
    json["groupid"] = group_id.getOriginValue();
    json["userid"] = self_id.getOriginValue();
    json["type"] = "added_group_verification";
    json["message"] = "";
    sendJsonToUser(adminID, std::move(json));
    return true;
}

GroupID User::createGroup()
{
    std::unique_lock lock(m_impl->m_user_group_set_mutex);
    GroupID groupid = serverManager.addGroupRoom(this->getUserID());
    m_impl->m_user_group_set.emplace(groupid);
    return groupid;
}

bool User::acceptGroup(GroupID group_id, UserID user_id)
{
    UserID self_id = this->getUserID();
    if (!serverManager.hasGroupRoom(group_id) ||
        serverManager.getGroupRoom(group_id)->hasMember(user_id) ||
        self_id != serverManager.getGroupRoom(group_id)->getAdministrator())
            return false;

    serverManager.getServerVerificationManager().acceptGroupRoom(user_id, group_id);

    // notify the other successfully adding a group
    qjson::JObject json(qjson::JValueType::JDict);
    json["groupid"] = group_id.getOriginValue();
    json["type"] = "added_group";
    sendJsonToUser(user_id, std::move(json));
    return true;
}

bool User::rejectGroup(GroupID group_id, UserID user_id)
{
    UserID self_id = this->getUserID();
    if (!serverManager.hasGroupRoom(group_id) ||
        serverManager.getGroupRoom(group_id)->hasMember(user_id) ||
        self_id != serverManager.getGroupRoom(group_id)->getAdministrator())
            return false;

    auto& ver = serverManager.getServerVerificationManager();
    if (!ver.hasGroupRoomVerification(user_id, group_id))
        return false;
    ver.rejectGroupRoom(user_id, group_id);
    
    qjson::JObject json(qjson::JValueType::JDict);
    json["groupid"] = group_id.getOriginValue();
    json["type"] = "rejected_to_add_group";
    sendJsonToUser(user_id, std::move(json));
    return true;
}

bool User::removeGroup(GroupID group_id)
{
    UserID self_id = this->getUserID();
    if (!serverManager.hasGroupRoom(group_id) ||
        self_id != serverManager.getGroupRoom(group_id)->getAdministrator())
        return false;

    auto user_list = serverManager.getGroupRoom(group_id)->getUserList();
    // notify all users in the room...

    try {
        serverManager.removeGroupRoom(group_id);
    } catch(...) {
        return false;
    }
    return true;
}

bool User::leaveGroup(GroupID group_id)
{
    UserID self_id = this->getUserID();

    if (!serverManager.hasGroupRoom(group_id))
        return false;

    auto group = serverManager.getGroupRoom(group_id);
    if (!group->removeMember(self_id) || group->getAdministrator() == self_id)
        return false;

    qjson::JObject json;
    json["type"] = "group_leave_member";
    json["data"]["user_id"] = self_id.getOriginValue();
    json["data"]["group_id"] = group_id.getOriginValue();

    auto list = group->getOperatorList();
    sendJsonToUser(group->getAdministrator(), qjson::JObject(json));
    sendJsonToUser(list.begin(), list.end(), std::move(json));
    return true;
}

void User::removeGroupVerification(GroupID group_id, UserID user_id)
{
    std::unique_lock
        lock(m_impl->m_user_group_verification_map_mutex);
    std::size_t size = m_impl->m_user_group_verification_map.count(group_id);
    if (!size) throw std::system_error(qls_errc::verification_not_existed);

    auto itor = m_impl->m_user_group_verification_map.find(group_id);
    for (; itor->first == group_id && itor != m_impl->m_user_group_verification_map.cend(); itor++) {
        if (itor->second.user_id == user_id) {
            m_impl->m_user_group_verification_map.erase(itor);
            break;
        }
    }
}

std::multimap<GroupID, Verification::GroupVerification>
    User::getGroupVerificationList() const
{
    std::shared_lock
        lock(m_impl->m_user_group_verification_map_mutex);
    return m_impl->m_user_group_verification_map;
}

void User::addConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr,
    DeviceType type)
{
    std::unique_lock
        lock(m_impl->m_connection_map_mutex);
    if (m_impl->m_connection_map.find(connection_ptr) != m_impl->m_connection_map.cend())
        throw std::system_error(qls_errc::socket_pointer_existed);

    m_impl->m_connection_map.emplace(connection_ptr, type);
}

bool User::hasConnection(const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr) const
{
    std::shared_lock lock(m_impl->m_connection_map_mutex);
    return m_impl->m_connection_map.find(connection_ptr) != m_impl->m_connection_map.cend();
}

void User::modifyConnectionType(const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr, DeviceType type)
{
    std::unique_lock lock(m_impl->m_connection_map_mutex);
    auto iter = m_impl->m_connection_map.find(connection_ptr);
    if (iter == m_impl->m_connection_map.cend())
        throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");

    iter->second = type;
}

void User::removeConnection(const std::shared_ptr<Connection<asio::ip::tcp::socket>>& connection_ptr)
{
    std::unique_lock lock(m_impl->m_connection_map_mutex);
    auto iter = m_impl->m_connection_map.find(connection_ptr);
    if (iter == m_impl->m_connection_map.cend())
        throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");
    
    m_impl->m_connection_map.erase(iter);
}

void User::notifyAll(std::string_view data)
{
    std::shared_lock lock(m_impl->m_connection_map_mutex);
    std::shared_ptr<std::string> buffer_ptr(std::allocate_shared<std::string>(
        std::pmr::polymorphic_allocator<std::string>(&local_user_sync_pool), data));
    for (const auto& [connection_ptr, type]: m_impl->m_connection_map) {
        asio::async_write(connection_ptr->socket, asio::buffer(*buffer_ptr),
            asio::bind_executor(connection_ptr->strand,
                [buffer_ptr](std::error_code ec, std::size_t) {
                    if (ec)
                        serverLogger.error('[', ec.category().name(), ']', ec.message());
            }));
    }
}

void User::notifyWithType(DeviceType type, std::string_view data)
{
    std::shared_lock lock(m_impl->m_connection_map_mutex);
    std::shared_ptr<std::string> buffer_ptr(std::allocate_shared<std::string>(
        std::pmr::polymorphic_allocator<std::string>(&local_user_sync_pool), data));
    for (const auto& [connection_ptr, dtype]: m_impl->m_connection_map) {
        if (dtype == type) {
            asio::async_write(connection_ptr->socket, asio::buffer(*buffer_ptr),
                asio::bind_executor(connection_ptr->strand,
                [this, buffer_ptr](std::error_code ec, std::size_t n) {
                    if (ec)
                        serverLogger.error('[', ec.category().name(), ']', ec.message());
            }));
        }
    }
}

void UserImplDeleter::operator()(UserImpl *user_impl)
{
    std::pmr::polymorphic_allocator<UserImpl>(&local_user_sync_pool)
        .delete_object(user_impl);
}

} // namespace qls
