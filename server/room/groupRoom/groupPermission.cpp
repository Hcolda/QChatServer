#include "groupPermission.h"

#include <algorithm>
#include <format>
#include <mutex>
#include <system_error>

#include "definition.hpp"
#include "qls_error.h"
#include "userid.hpp"

namespace qls {

void GroupPermission::modifyPermission(std::string_view permissionName,
                                       PermissionType type) {
  std::lock_guard<std::shared_mutex> lock(m_permission_map_mutex);
  m_permission_map.emplace(permissionName, type);
}

void GroupPermission::removePermission(std::string_view permissionName) {
  std::lock_guard<std::shared_mutex> lock(m_permission_map_mutex);

  // 是否有此权限
  auto itor = m_permission_map.find(permissionName);
  if (itor == m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  m_permission_map.erase(itor);
}

PermissionType
GroupPermission::getPermissionType(std::string_view permissionName) const {
  std::shared_lock lock(m_permission_map_mutex);

  // 是否有此权限
  auto itor = m_permission_map.find(permissionName);
  if (itor == m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  return itor->second;
}

std::unordered_map<std::string, PermissionType, string_hash, std::equal_to<>>
GroupPermission::getPermissionList() const {
  std::shared_lock lock(m_permission_map_mutex);
  return m_permission_map;
}

void GroupPermission::modifyUserPermission(const UserID &user_id,
                                           PermissionType type) {
  std::lock_guard<std::shared_mutex> lock(m_user_permission_map_mutex);
  m_user_permission_map[user_id] = type;
}

void GroupPermission::removeUser(const UserID &user_id) {
  std::lock_guard<std::shared_mutex> lock(m_user_permission_map_mutex);

  // 是否有此user
  auto itor = m_user_permission_map.find(user_id);
  if (itor == m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  m_user_permission_map.erase(itor);
}

bool GroupPermission::userHasPermission(const UserID &user_id,
                                        std::string_view permissionName) const {
  std::shared_lock lock1(m_permission_map_mutex, std::defer_lock);
  std::shared_lock lock2(m_user_permission_map_mutex, std::defer_lock);
  std::lock(lock1, lock2);

  // 是否有此user
  auto itor = m_user_permission_map.find(user_id);
  if (itor == m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  // 是否有此权限
  auto itor2 = m_permission_map.find(permissionName);
  if (itor2 == m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  // 返回权限
  return itor->second >= itor2->second;
}

PermissionType
GroupPermission::getUserPermissionType(const UserID &user_id) const {
  std::shared_lock lock(m_user_permission_map_mutex);

  // 是否有此user
  auto itor = m_user_permission_map.find(user_id);
  if (itor == m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  return itor->second;
}

std::unordered_map<UserID, PermissionType>
GroupPermission::getUserPermissionList() const {
  std::shared_lock lock(m_user_permission_map_mutex);
  return m_user_permission_map;
}

std::vector<UserID> GroupPermission::getDefaultUserList() const {
  std::shared_lock lock(m_user_permission_map_mutex);

  std::vector<UserID> return_vector;
  std::for_each(
      m_user_permission_map.cbegin(), m_user_permission_map.cend(),
      [&return_vector](const std::pair<UserID, PermissionType> &pair) {
        if (pair.second == PermissionType::Default) {
          return_vector.push_back(pair.first);
        }
      });

  return return_vector;
}

std::vector<UserID> GroupPermission::getOperatorList() const {
  std::shared_lock lock(m_user_permission_map_mutex);

  std::vector<UserID> return_vector;
  std::for_each(
      m_user_permission_map.cbegin(), m_user_permission_map.cend(),
      [&return_vector](const std::pair<UserID, PermissionType> &pair) {
        if (pair.second == PermissionType::Operator) {
          return_vector.push_back(pair.first);
        }
      });

  return return_vector;
}

std::vector<UserID> GroupPermission::getAdministratorList() const {
  std::shared_lock lock(m_user_permission_map_mutex);

  std::vector<UserID> return_vector;
  std::for_each(
      m_user_permission_map.cbegin(), m_user_permission_map.cend(),
      [&return_vector](const std::pair<UserID, PermissionType> &pair) {
        if (pair.second == PermissionType::Administrator) {
          return_vector.push_back(pair.first);
        }
      });

  return return_vector;
}

} // namespace qls
