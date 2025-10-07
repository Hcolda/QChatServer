#include "groupPermission.h"

#include <format>
#include <functional>
#include <memory_resource>
#include <mutex>
#include <shared_mutex>
#include <system_error>

#include "definition.hpp"
#include "qls_error.h"
#include "userid.hpp"

namespace qls {

struct GroupPermission::GroupPermissionImpl {
  std::pmr::unordered_map<std::string, PermissionType, string_hash,
                          std::equal_to<>>
      m_permission_map; ///< Map of permissions and their types
  mutable std::shared_mutex
      m_permission_map_mutex; ///< Mutex for thread-safe access to permission
                              ///< map

  std::pmr::unordered_map<UserID, PermissionType>
      m_user_permission_map; ///< Map of users and their permission types
  mutable std::shared_mutex
      m_user_permission_map_mutex; ///< Mutex for thread-safe access to user
  ///< permission map
  std::pmr::memory_resource *m_local_memory_resource;

  GroupPermissionImpl(std::pmr::memory_resource *memory_resource)
      : m_local_memory_resource(memory_resource),
        m_permission_map(memory_resource),
        m_user_permission_map(memory_resource) {}
};

void GroupPermission::GroupPermissionImplDeleter::operator()(
    GroupPermissionImpl *gpi) {
  std::pmr::polymorphic_allocator<GroupPermissionImpl>{memory_resource}
      .delete_object(gpi);
}

GroupPermission::GroupPermission(std::pmr::memory_resource *memory_resource)
    : m_impl(
          std::pmr::polymorphic_allocator<GroupPermissionImpl>{memory_resource}
              .new_object<GroupPermissionImpl>(memory_resource),
          {memory_resource}) {}

GroupPermission::~GroupPermission() noexcept = default;

void GroupPermission::modifyPermission(std::string_view permissionName,
                                       PermissionType type) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_permission_map_mutex);
  m_impl->m_permission_map.emplace(permissionName, type);
}

void GroupPermission::removePermission(std::string_view permissionName) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_permission_map_mutex);

  // 是否有此权限
  auto itor = m_impl->m_permission_map.find(permissionName);
  if (itor == m_impl->m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  m_impl->m_permission_map.erase(itor);
}

PermissionType
GroupPermission::getPermissionType(std::string_view permissionName) const {
  std::shared_lock lock(m_impl->m_permission_map_mutex);

  // 是否有此权限
  auto itor = m_impl->m_permission_map.find(permissionName);
  if (itor == m_impl->m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  return itor->second;
}

void GroupPermission::getPermissionList(
    const std::function<void(
        const std::pmr::unordered_map<std::string, PermissionType, string_hash,
                                      std::equal_to<>> &)> &func) const {
  std::shared_lock lock(m_impl->m_permission_map_mutex);
  std::invoke(func, m_impl->m_permission_map);
}

void GroupPermission::modifyUserPermission(const UserID &user_id,
                                           PermissionType type) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_user_permission_map_mutex);
  m_impl->m_user_permission_map[user_id] = type;
}

void GroupPermission::removeUser(const UserID &user_id) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_user_permission_map_mutex);

  // 是否有此user
  auto itor = m_impl->m_user_permission_map.find(user_id);
  if (itor == m_impl->m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  m_impl->m_user_permission_map.erase(itor);
}

bool GroupPermission::userHasPermission(const UserID &user_id,
                                        std::string_view permissionName) const {
  std::shared_lock lock1(m_impl->m_permission_map_mutex, std::defer_lock);
  std::shared_lock lock2(m_impl->m_user_permission_map_mutex, std::defer_lock);
  std::lock(lock1, lock2);

  // 是否有此user
  auto itor = m_impl->m_user_permission_map.find(user_id);
  if (itor == m_impl->m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  // 是否有此权限
  auto itor2 = m_impl->m_permission_map.find(permissionName);
  if (itor2 == m_impl->m_permission_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::no_permission),
                            std::format("no permission: {}", permissionName));
  }

  // 返回权限
  return itor->second >= itor2->second;
}

PermissionType
GroupPermission::getUserPermissionType(const UserID &user_id) const {
  std::shared_lock lock(m_impl->m_user_permission_map_mutex);

  // 是否有此user
  auto itor = m_impl->m_user_permission_map.find(user_id);
  if (itor == m_impl->m_user_permission_map.cend()) {
    throw std::system_error(
        make_error_code(qls_errc::user_not_existed),
        std::format("no user: {}", user_id.getOriginValue()));
  }

  return itor->second;
}

void GroupPermission::getUserPermissionList(
    const std::function<void(
        const std::pmr::unordered_map<UserID, PermissionType> &)> &func) const {
  std::shared_lock lock(m_impl->m_user_permission_map_mutex);
  std::invoke(func, m_impl->m_user_permission_map);
}

} // namespace qls
