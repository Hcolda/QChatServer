#include "manager.h"

#include <Ini.h>
#include <memory_resource>
#include <shared_mutex>
#include <system_error>

#include "groupid.hpp"
#include "qls_error.h"
#include "user.h"

extern qini::INIObject serverIni;

namespace qls {

struct ManagerImpl {
  DataManager m_dataManager;                 ///< Data manager instance.
  VerificationManager m_verificationManager; ///< Verification manager instance.

  // Group room map
  std::pmr::synchronized_pool_resource m_groupRoom_sync_pool;
  std::pmr::unordered_map<GroupID, std::shared_ptr<GroupRoom>> m_groupRoom_map{
      &m_groupRoom_sync_pool}; ///< Map of group room IDs to group rooms.
  std::shared_mutex m_groupRoom_map_mutex; ///< Mutex for group room map.

  // Private room map
  std::pmr::synchronized_pool_resource m_privateRoom_sync_pool;
  std::pmr::unordered_map<GroupID, std::shared_ptr<PrivateRoom>>
      m_privateRoom_map{&m_privateRoom_sync_pool}; ///< Map of private room IDs
                                                   ///< to private rooms.
  std::shared_mutex m_privateRoom_map_mutex; ///< Mutex for private room map.

  // Map of user IDs to private room IDs
  std::pmr::unordered_map<PrivateRoomIDStruct, GroupID,
                          PrivateRoomIDStructHasher>
      m_userID_to_privateRoomID_map{&m_privateRoom_sync_pool};
  std::shared_mutex m_userID_to_privateRoomID_map_mutex;

  // User map
  std::pmr::synchronized_pool_resource m_user_sync_pool;
  std::pmr::unordered_map<UserID, std::shared_ptr<User>> m_user_map{
      &m_user_sync_pool};
  std::shared_mutex m_user_map_mutex;

  std::unordered_map<std::shared_ptr<Connection<asio::ip::tcp::socket>>, UserID>
      m_connection_map;
  std::shared_mutex m_connection_map_mutex;

  // New user ID
  std::atomic<long long> m_newUserId;
  // New private room ID
  std::atomic<long long> m_newPrivateRoomId;
  // New group room ID
  std::atomic<long long> m_newGroupRoomId;

  // SQL process manager
  SQLDBProcess m_sqlProcess;

  // Network
  std::pmr::synchronized_pool_resource m_network_sync_pool;
  Network m_network{&m_network_sync_pool};
};

Manager::Manager() : m_impl(std::make_unique<ManagerImpl>()) {}

Manager::~Manager() = default;

void Manager::init() {
  // initiate sql database connection

  // m_sqlProcess.setSQLServerInfo(serverIni["mysql"]["username"],
  //     serverIni["mysql"]["password"],
  //     "mysql",
  //     serverIni["mysql"]["host"],
  //     unsigned short(std::stoi(serverIni["mysql"]["port"])));

  // m_sqlProcess.connectSQLServer();

  {
    m_impl->m_newUserId = 10000;
    m_impl->m_newPrivateRoomId = 10000;
    m_impl->m_newGroupRoomId = 10000;

    // initiate data from sql database
    // sql更新初始化数据
    // ...
  }

  m_impl->m_dataManager.init();
  m_impl->m_verificationManager.init();
}

GroupID Manager::addPrivateRoom(const UserID &user1_id,
                                const UserID &user2_id) {
  std::unique_lock lock1(m_impl->m_privateRoom_map_mutex, std::defer_lock);
  std::unique_lock lock2(m_impl->m_userID_to_privateRoomID_map_mutex,
                         std::defer_lock);
  std::lock(lock1, lock2);

  // 私聊房间id
  GroupID privateRoom_id(m_impl->m_newGroupRoomId++);
  {
    // Update database
    /**
     * 这里有申请sql 创建私聊房间等命令
     */
  }

  m_impl->m_privateRoom_map[privateRoom_id] = std::allocate_shared<PrivateRoom>(
      std::pmr::polymorphic_allocator<PrivateRoom>(
          &m_impl->m_privateRoom_sync_pool),
      user1_id, user2_id, true, &m_impl->m_privateRoom_sync_pool);
  m_impl->m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;

  return privateRoom_id;
}

GroupID Manager::getPrivateRoomId(const UserID &user1_id,
                                  const UserID &user2_id) const {
  std::shared_lock lock(m_impl->m_userID_to_privateRoomID_map_mutex);
  if (m_impl->m_userID_to_privateRoomID_map.find({user1_id, user2_id}) !=
      m_impl->m_userID_to_privateRoomID_map.cend()) {
    return GroupID(
        m_impl->m_userID_to_privateRoomID_map.find({user1_id, user2_id})
            ->second);
  }
  if (m_impl->m_userID_to_privateRoomID_map.find({user2_id, user1_id}) !=
      m_impl->m_userID_to_privateRoomID_map.cend()) {
    return GroupID(
        m_impl->m_userID_to_privateRoomID_map.find({user2_id, user1_id})
            ->second);
  }
  throw std::system_error(make_error_code(qls_errc::private_room_not_existed));
}

bool Manager::hasPrivateRoom(const GroupID &private_room_id) const {
  std::shared_lock lock(m_impl->m_privateRoom_map_mutex);
  return m_impl->m_privateRoom_map.find(private_room_id) !=
         m_impl->m_privateRoom_map.cend();
}

bool Manager::hasPrivateRoom(const UserID &user1_id,
                             const UserID &user2_id) const {
  std::shared_lock lock(m_impl->m_userID_to_privateRoomID_map_mutex);
  if (m_impl->m_userID_to_privateRoomID_map.find({user1_id, user2_id}) !=
      m_impl->m_userID_to_privateRoomID_map.cend()) {
    return true;
  }
  if (m_impl->m_userID_to_privateRoomID_map.find({user2_id, user1_id}) !=
      m_impl->m_userID_to_privateRoomID_map.cend()) {
    return true;
  }
  return false;
}

std::shared_ptr<PrivateRoom>
Manager::getPrivateRoom(const GroupID &private_room_id) const {
  std::shared_lock lock(m_impl->m_privateRoom_map_mutex);
  auto itor = m_impl->m_privateRoom_map.find(private_room_id);
  if (itor == m_impl->m_privateRoom_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_not_existed));
  }
  return itor->second;
}

void Manager::removePrivateRoom(const GroupID &private_room_id) {
  std::unique_lock lock1(m_impl->m_privateRoom_map_mutex, std::defer_lock);
  std::unique_lock lock2(m_impl->m_userID_to_privateRoomID_map_mutex,
                         std::defer_lock);
  std::lock(lock1, lock2);

  auto itor = m_impl->m_privateRoom_map.find(private_room_id);
  if (itor == m_impl->m_privateRoom_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_not_existed));
  }

  {
    /*
     * 这里有申请sql 删除私聊房间等命令
     */
  }
  auto [user1_id, user2_id] = itor->second->getUserID();

  if (m_impl->m_userID_to_privateRoomID_map.find({user1_id, user2_id}) !=
      m_impl->m_userID_to_privateRoomID_map.cend()) {
    m_impl->m_userID_to_privateRoomID_map.erase({user1_id, user2_id});
  } else if (m_impl->m_userID_to_privateRoomID_map.find({user2_id, user1_id}) !=
             m_impl->m_userID_to_privateRoomID_map.cend()) {
    m_impl->m_userID_to_privateRoomID_map.erase({user2_id, user1_id});
  }

  m_impl->m_privateRoom_map.erase(itor);
}

GroupID Manager::addGroupRoom(const UserID &operator_user_id) {
  std::unique_lock lock(m_impl->m_groupRoom_map_mutex);
  // 新群聊id
  GroupID group_room_id(m_impl->m_newPrivateRoomId++);
  {
    /*
     * sql 创建群聊获取群聊id
     */
  }

  m_impl->m_groupRoom_map[group_room_id] = std::allocate_shared<GroupRoom>(
      std::pmr::polymorphic_allocator<GroupRoom>(
          &m_impl->m_groupRoom_sync_pool),
      group_room_id, operator_user_id, true, &m_impl->m_groupRoom_sync_pool);

  return group_room_id;
}

bool Manager::hasGroupRoom(const GroupID &group_room_id) const {
  std::shared_lock lock(m_impl->m_groupRoom_map_mutex);
  return m_impl->m_groupRoom_map.find(group_room_id) !=
         m_impl->m_groupRoom_map.cend();
}

std::shared_ptr<GroupRoom>
Manager::getGroupRoom(const GroupID &group_room_id) const {
  std::shared_lock lock(m_impl->m_groupRoom_map_mutex);
  auto itor = m_impl->m_groupRoom_map.find(group_room_id);
  if (itor == m_impl->m_groupRoom_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::group_room_not_existed));
  }
  return itor->second;
}

void Manager::removeGroupRoom(const GroupID &group_room_id) {
  std::unique_lock lock(m_impl->m_groupRoom_map_mutex);
  auto itor = m_impl->m_groupRoom_map.find(group_room_id);
  if (itor == m_impl->m_groupRoom_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::group_room_not_existed));
  }

  {
    // Remove the group room data from database
    /*
     * sql删除群聊
     */
  }

  m_impl->m_groupRoom_map.erase(group_room_id);
}

std::shared_ptr<User> Manager::addNewUser() {
  std::unique_lock lock(m_impl->m_user_map_mutex);
  UserID newUserId(m_impl->m_newUserId++);
  {
    // Update data from database
    // sql处理数据
  }

  auto [iter, _] = m_impl->m_user_map.emplace(
      newUserId,
      std::allocate_shared<User>(
          std::pmr::polymorphic_allocator<User>(&m_impl->m_user_sync_pool),
          newUserId, true, &m_impl->m_user_sync_pool));
  return iter->second;
}

bool Manager::hasUser(const UserID &user_id) const {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  return m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend();
}

std::shared_ptr<User> Manager::getUser(const UserID &user_id) const {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  auto itor = m_impl->m_user_map.find(user_id);
  if (itor == m_impl->m_user_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::user_not_existed));
  }

  return itor->second;
}

void Manager::getUserList(
    const std::function<void(
        const std::pmr::unordered_map<UserID, std::shared_ptr<qls::User>> &)>
        &func) const {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  std::invoke(func, m_impl->m_user_map);
}

void Manager::registerConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr) {
  std::unique_lock lock(m_impl->m_connection_map_mutex);
  if (m_impl->m_connection_map.find(connection_ptr) !=
      m_impl->m_connection_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::socket_pointer_existed));
  }
  m_impl->m_connection_map.emplace(connection_ptr, -1LL);
}

bool Manager::hasConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr)
    const {
  std::shared_lock lock(m_impl->m_connection_map_mutex);
  return m_impl->m_connection_map.find(connection_ptr) !=
         m_impl->m_connection_map.cend();
}

bool Manager::matchUserOfConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr,
    const UserID &user_id) const {
  std::shared_lock lock(m_impl->m_connection_map_mutex);
  auto iter = m_impl->m_connection_map.find(connection_ptr);
  if (iter == m_impl->m_connection_map.cend()) {
    return false;
  }
  return iter->second == user_id;
}

UserID Manager::getUserIDOfConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr)
    const {
  std::shared_lock lock(m_impl->m_connection_map_mutex);
  auto iter = m_impl->m_connection_map.find(connection_ptr);
  if (iter == m_impl->m_connection_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::socket_pointer_not_existed));
  }
  return iter->second;
}

void Manager::modifyUserOfConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr,
    const UserID &user_id, DeviceType type) {
  std::unique_lock lock1(m_impl->m_connection_map_mutex, std::defer_lock);
  std::shared_lock lock2(m_impl->m_user_map_mutex, std::defer_lock);
  std::lock(lock1, lock2);

  if (m_impl->m_user_map.find(user_id) == m_impl->m_user_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::user_not_existed));
  }

  auto iter = m_impl->m_connection_map.find(connection_ptr);
  if (iter == m_impl->m_connection_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::socket_pointer_not_existed));
  }

  if (iter->second != -1LL) {
    m_impl->m_user_map.find(iter->second)
        ->second->removeConnection(connection_ptr);
  }
  m_impl->m_user_map.find(user_id)->second->addConnection(connection_ptr, type);
  iter->second = user_id;
}

void Manager::removeConnection(
    const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr) {
  std::unique_lock lock1(m_impl->m_connection_map_mutex, std::defer_lock);
  std::shared_lock lock2(m_impl->m_user_map_mutex, std::defer_lock);
  std::lock(lock1, lock2);

  auto iter = m_impl->m_connection_map.find(connection_ptr);
  if (iter == m_impl->m_connection_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::socket_pointer_not_existed));
  }

  if (iter->second != -1LL) {
    m_impl->m_user_map.find(iter->second)
        ->second->removeConnection(connection_ptr);
  }

  m_impl->m_connection_map.erase(iter);
}

SQLDBProcess &Manager::getServerSqlProcess() { return m_impl->m_sqlProcess; }

DataManager &Manager::getServerDataManager() { return m_impl->m_dataManager; }

VerificationManager &Manager::getServerVerificationManager() {
  return m_impl->m_verificationManager;
}

qls::Network &Manager::getServerNetwork() { return m_impl->m_network; }

} // namespace qls
