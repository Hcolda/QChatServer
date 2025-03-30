#include "groupRoom.h"

#include <stdexcept>
#include <algorithm>
#include <format>
#include <memory>
#include <system_error>
#include <memory_resource>
#include <unordered_map>
#include <map>
#include <shared_mutex>
#include <atomic>

#include <Json.h>

#include "manager.h"
#include "qls_error.h"
#include "returnStateMessage.hpp"

extern qls::Manager serverManager;

namespace qls
{

static std::pmr::synchronized_pool_resource local_sync_group_room_pool;

struct GroupRoomImpl
{
    GroupID                 m_group_id;
    UserID                  m_administrator_user_id;
    std::shared_mutex       m_administrator_user_id_mutex;
    std::atomic<bool>       m_can_be_used;
    GroupPermission         m_permission;

    std::unordered_map<UserID, GroupRoom::UserDataStructure>
                            m_user_id_map;
    std::shared_mutex       m_user_id_map_mutex;

    std::unordered_map<UserID,
        std::pair<std::chrono::utc_clock::time_point,
        std::chrono::minutes>>
                            m_muted_user_map;
    std::shared_mutex       m_muted_user_map_mutex;

    std::map<std::chrono::utc_clock::time_point,
        MessageStructure>
                            m_message_map;
    std::shared_mutex       m_message_map_mutex;

    asio::steady_timer      m_clear_timer{serverManager.getServerNetwork().get_io_context()};
};

void GroupRoomImplDeleter::operator()(GroupRoomImpl *gri)
{
    local_sync_group_room_pool.deallocate(gri, sizeof(GroupRoomImpl));
}

// GroupRoom
GroupRoom::GroupRoom(GroupID group_id, UserID administrator, bool is_create):
    TextDataRoom(&local_sync_group_room_pool),
    m_impl(
        [](GroupRoomImpl* gi) {
            new(gi) GroupRoomImpl(); return gi;
            } (static_cast<GroupRoomImpl*>(
                local_sync_group_room_pool.allocate(sizeof(GroupRoomImpl)))))
{
    m_impl->m_group_id = group_id;
    m_impl->m_administrator_user_id = administrator;

    if (is_create) {
        // 创建群聊 sql
        m_impl->m_can_be_used = true;
    } else {
        // 加载群聊 sql
        m_impl->m_can_be_used = true;
    }

    TextDataRoom::joinRoom(administrator);
    asio::co_spawn(serverManager.getServerNetwork().get_io_context(),
        auto_clean(), asio::detached);
}

GroupRoom::~GroupRoom() noexcept
{
    stop_cleaning();
}

bool GroupRoom::addMember(UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    {
        std::lock_guard<std::shared_mutex> lg(m_impl->m_user_id_map_mutex);
        if (m_impl->m_user_id_map.find(user_id) == m_impl->m_user_id_map.cend())
            m_impl->m_user_id_map.emplace(user_id, serverManager.getUser(user_id)->getUserName());
    }
    TextDataRoom::joinRoom(user_id);

    return true;
}

bool GroupRoom::hasMember(UserID user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    std::lock_guard<std::shared_mutex> lg(m_impl->m_user_id_map_mutex);
    return m_impl->m_user_id_map.find(user_id) != m_impl->m_user_id_map.cend();
}

bool GroupRoom::removeMember(UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    {
        std::lock_guard<std::shared_mutex> lg(m_impl->m_user_id_map_mutex);
        if (m_impl->m_user_id_map.find(user_id) != m_impl->m_user_id_map.cend())
            m_impl->m_user_id_map.erase(user_id);
    }
    TextDataRoom::leaveRoom(user_id);

    return true;
}

void GroupRoom::sendMessage(UserID sender_user_id, std::string_view message)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    // 是否有此user_id
    if (!hasUser(sender_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.cend()) {
            if (itor->second.first + itor->second.second >= std::chrono::utc_clock::now())
                return;
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    // store the message
    {
        std::unique_lock<std::shared_mutex> lock(m_impl->m_message_map_mutex);
        auto time_point = std::chrono::utc_clock::now();
        while (m_impl->m_message_map.find(time_point) != m_impl->m_message_map.cend()) {
            ++time_point;
        }
        m_impl->m_message_map.insert({
            time_point,
            {sender_user_id, std::string(message),
            MessageType::NOMAL_MESSAGE} });
    }

    qjson::JObject json;
    json["type"] = "group_message";
    json["data"]["user_id"] = sender_user_id.getOriginValue();
    json["data"]["group_id"] = m_impl->m_group_id.getOriginValue();
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json));
}

void GroupRoom::sendTipMessage(UserID sender_user_id,
    std::string_view message)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    // 是否有此user_id
    if (!hasUser(sender_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.cend()) {
            if (itor->second.first + itor->second.second >= std::chrono::utc_clock::now())
                return;
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    // store the message
    {
        std::unique_lock<std::shared_mutex> lock(m_impl->m_message_map_mutex);
        auto time_point = std::chrono::utc_clock::now();
        while (m_impl->m_message_map.find(time_point) != m_impl->m_message_map.cend()) {
            ++time_point;
        }
        m_impl->m_message_map.insert({
            time_point,
            {sender_user_id, std::string(message),
            MessageType::TIP_MESSAGE} });
    }

    qjson::JObject json;
    json["type"] = "group_tip_message";
    json["data"]["user_id"] = sender_user_id.getOriginValue();
    json["data"]["group_id"] = m_impl->m_group_id.getOriginValue();
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json));
}

void GroupRoom::sendUserTipMessage(UserID sender_user_id,
    std::string_view message, UserID receiver_user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    // 是否有此user_id
    if (!hasUser(receiver_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.cend()) {
            if (itor->second.first + itor->second.second >= std::chrono::utc_clock::now())
                return;
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    // store the message
    {
        std::unique_lock<std::shared_mutex> lock(m_impl->m_message_map_mutex);
        auto time_point = std::chrono::utc_clock::now();
        while (m_impl->m_message_map.find(time_point) != m_impl->m_message_map.cend()) {
            ++time_point;
        }
        m_impl->m_message_map.insert({
            time_point,
            {
                sender_user_id, std::string(message),
                MessageType::TIP_MESSAGE,
                receiver_user_id
            }});
    }

    qjson::JObject json;
    json["type"] = "group_tip_message";
    json["data"]["user_id"] = sender_user_id.getOriginValue();
    json["data"]["group_id"] = m_impl->m_group_id.getOriginValue();
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json), receiver_user_id);
}

std::vector<MessageResult> GroupRoom::getMessage(
    const std::chrono::utc_clock::time_point& from,
    const std::chrono::utc_clock::time_point& to)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (from > to)
        return {};

    std::shared_lock<std::shared_mutex> lock(m_impl->m_message_map_mutex);
    const auto& message_map = std::as_const(m_impl->m_message_map);
    if (message_map.empty())
        return {};
    auto start = message_map.lower_bound(from);
    auto end = message_map.upper_bound(to);
    std::vector<MessageResult> revec;
    for (; start != end; ++start) {
        revec.emplace_back(start->first, start->second);
    }
    return revec;
}

bool GroupRoom::hasUser(UserID user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    return m_impl->m_user_id_map.find(user_id) != m_impl->m_user_id_map.cend();
}

std::unordered_map<UserID, GroupRoom::UserDataStructure> GroupRoom::getUserList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    return m_impl->m_user_id_map;
}

std::string GroupRoom::getUserNickname(UserID user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    auto itor = m_impl->m_user_id_map.find(user_id);
    if (itor == m_impl->m_user_id_map.cend())
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "user isn't in the room");

    return itor->second.nickname;
}

long long GroupRoom::getUserGroupLevel(UserID user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    auto itor = m_impl->m_user_id_map.find(user_id);
    if (itor == m_impl->m_user_id_map.cend())
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "user isn't in the room");

    return itor->second.level.getValue();
}

std::unordered_map<UserID, PermissionType>
    GroupRoom::getUserPermissionList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    return std::move(m_impl->m_permission.getUserPermissionList());
}

UserID GroupRoom::getAdministrator() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::shared_lock<std::shared_mutex> lock(m_impl->m_administrator_user_id_mutex);
    return m_impl->m_administrator_user_id;
}

void GroupRoom::setAdministrator(UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));

    std::unique_lock<std::shared_mutex> lock1(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::unique_lock<std::shared_mutex> lock2(m_impl->m_administrator_user_id_mutex, std::defer_lock);
    std::lock(lock1, lock2);

    if (m_impl->m_administrator_user_id == 0) {
        auto itor = m_impl->m_user_id_map.find(user_id);
        if (itor == m_impl->m_user_id_map.cend()) {
            m_impl->m_user_id_map.emplace(user_id, serverManager.getUser(user_id)->getUserName());
            m_impl->m_permission.modifyUserPermission(user_id,
                PermissionType::Administrator);
        }
        else
            m_impl->m_permission.modifyUserPermission(user_id,
                PermissionType::Administrator);
    }
    else
    {
        m_impl->m_permission.modifyUserPermission(m_impl->m_administrator_user_id,
            PermissionType::Default);
        m_impl->m_permission.modifyUserPermission(user_id,
            PermissionType::Administrator);
        m_impl->m_administrator_user_id = user_id;
    }
}

GroupID GroupRoom::getGroupID() const
{
    return m_impl->m_group_id;
}

std::vector<UserID> GroupRoom::getDefaultUserList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    return std::move(m_impl->m_permission.getDefaultUserList());
}

std::vector<UserID> GroupRoom::getOperatorList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    return std::move(m_impl->m_permission.getOperatorList());
}

bool GroupRoom::muteUser(UserID executor_id, UserID user_id, const std::chrono::minutes& mins)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (executor_id == user_id || !hasUser(user_id) || !hasUser(executor_id))
        return false;

    auto executor_idType = m_impl->m_permission.getUserPermissionType(executor_id);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executor_idType)
        return false;

    std::unique_lock<std::shared_mutex> lock1(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> lock2(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::lock(lock1, lock2);

    m_impl->m_muted_user_map[user_id] = std::pair<std::chrono::utc_clock::time_point,
        std::chrono::minutes>(std::chrono::utc_clock::now(), mins);
    sendTipMessage(executor_id, std::format("{} was muted by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executor_id].nickname));

    return true;
}

bool GroupRoom::unmuteUser(UserID executor_id, UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (executor_id == user_id || !hasUser(user_id) || !hasUser(executor_id))
        return false;

    auto executor_idType = m_impl->m_permission.getUserPermissionType(executor_id);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executor_idType)
        return false;

    std::unique_lock<std::shared_mutex> lock1(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> lock2(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::lock(lock1, lock2);

    m_impl->m_muted_user_map.erase(user_id);
    sendTipMessage(executor_id, std::format("{} was unmuted by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executor_id].nickname));

    return true;
}

bool GroupRoom::kickUser(UserID executor_id, UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (executor_id == user_id || !hasUser(user_id) || !hasUser(executor_id))
        return false;

    auto executor_idType = m_impl->m_permission.getUserPermissionType(executor_id);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executor_idType)
        return false;

    std::unique_lock<std::shared_mutex> lock1(m_impl->m_user_id_map_mutex, std::defer_lock),
        lock2(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::lock(lock1, lock2);
    sendTipMessage(executor_id, std::format("{} was kicked by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executor_id].nickname));
    m_impl->m_user_id_map.erase(user_id);

    return true;
}

bool GroupRoom::addOperator(UserID executor_id, UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (executor_id == user_id || !hasUser(user_id) || !hasUser(executor_id))
        return false;

    if (m_impl->m_permission.getUserPermissionType(executor_id) != PermissionType::Administrator)
        return false;
    if (m_impl->m_permission.getUserPermissionType(user_id) != PermissionType::Default)
        return false;

    m_impl->m_permission.modifyUserPermission(user_id,
        PermissionType::Operator);

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    sendTipMessage(executor_id, std::format("{} was turned operator by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executor_id].nickname));

    return true;
}

bool GroupRoom::removeOperator(UserID executor_id, UserID user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::group_room_unable_to_use));
    if (executor_id == user_id || !hasUser(user_id) || !hasUser(executor_id))
        return false;

    if (m_impl->m_permission.getUserPermissionType(executor_id) !=
        PermissionType::Administrator)
        return false;
    if (m_impl->m_permission.getUserPermissionType(user_id) !=
        PermissionType::Operator)
        return false;

    m_impl->m_permission.modifyUserPermission(user_id,
        PermissionType::Default);

    std::shared_lock<std::shared_mutex> lock(m_impl->m_user_id_map_mutex);
    sendTipMessage(executor_id, std::format("{} was turned default user by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executor_id].nickname));

    return true;
}

void GroupRoom::removeThisRoom()
{
    m_impl->m_can_be_used = false;

    {
        // sql 上面删除此房间
    }

    // 剩下其他东西
}

bool GroupRoom::canBeUsed() const
{
    return m_impl->m_can_be_used;
}

asio::awaitable<void> GroupRoom::auto_clean()
{
    using namespace std::chrono_literals;
    try {
        while (true) {
            m_impl->m_clear_timer.expires_after(10min);
            co_await m_impl->m_clear_timer.async_wait(asio::use_awaitable);
            std::unique_lock<std::shared_mutex> lock(m_impl->m_message_map_mutex);
            auto end = m_impl->m_message_map.upper_bound(std::chrono::utc_clock::now() - std::chrono::days(7));
            m_impl->m_message_map.erase(m_impl->m_message_map.begin(), end);
        }
    } catch(...) {
        co_return;
    }
}

void GroupRoom::stop_cleaning()
{
    std::error_code ec;
    m_impl->m_clear_timer.cancel(ec);
}

} // namespace qls
