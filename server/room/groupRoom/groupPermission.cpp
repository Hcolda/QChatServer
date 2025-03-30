#include "groupPermission.h"

#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <system_error>

#include "definition.hpp"
#include "qls_error.h"

namespace qls
{
    
void GroupPermission::modifyPermission(std::string_view permissionName, PermissionType type)
{
    std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);
    m_permission_map[std::string(permissionName)] = type;
}

void GroupPermission::removePermission(std::string_view permissionName)
{
    std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);

    // 是否有此权限
    auto itor = m_permission_map.find(permissionName);
    if (itor == m_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::no_permission), "no permission: " + std::string(permissionName));

    m_permission_map.erase(itor);
}

PermissionType GroupPermission::getPermissionType(std::string_view permissionName) const
{
    std::shared_lock<std::shared_mutex> lock(m_permission_map_mutex);

    // 是否有此权限
    auto itor = m_permission_map.find(permissionName);
    if (itor == m_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::no_permission), "no permission: " + std::string(permissionName));

    return itor->second;
}

std::unordered_map<std::string, PermissionType, string_hash, std::equal_to<>>
    GroupPermission::getPermissionList() const
{
    std::shared_lock<std::shared_mutex> lock(m_permission_map_mutex);
    return m_permission_map;
}

void GroupPermission::modifyUserPermission(UserID user_id, PermissionType type)
{
    std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);
    m_user_permission_map[user_id] = type;
}

void GroupPermission::removeUser(UserID user_id)
{
    std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);

    // 是否有此user
    auto itor = m_user_permission_map.find(user_id);
    if (itor == m_user_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "no user: " + std::to_string(user_id));

    m_user_permission_map.erase(itor);
}

bool GroupPermission::userHasPermission(UserID user_id, std::string_view permissionName) const
{
    std::shared_lock<std::shared_mutex> lock1(m_permission_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> lock2(m_user_permission_map_mutex, std::defer_lock);
    std::lock(lock1, lock2);

    // 是否有此user
    auto itor = m_user_permission_map.find(user_id);
    if (itor == m_user_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "no user: " + std::to_string(user_id));

    // 是否有此权限
    auto itor2 = m_permission_map.find(permissionName);
    if (itor2 == m_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::no_permission), "no permission: " + std::string(permissionName));

    // 返回权限
    return itor->second >= itor2->second;
}

PermissionType GroupPermission::getUserPermissionType(UserID user_id) const
{
    std::shared_lock<std::shared_mutex> lock(m_user_permission_map_mutex);

    // 是否有此user
    auto itor = m_user_permission_map.find(user_id);
    if (itor == m_user_permission_map.cend())
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "no user: " + std::to_string(user_id));

    return itor->second;
}

std::unordered_map<UserID, PermissionType>
    GroupPermission::getUserPermissionList() const
{
    std::shared_lock<std::shared_mutex> lock(m_user_permission_map_mutex);
    return m_user_permission_map;
}

std::vector<UserID> GroupPermission::getDefaultUserList() const
{
    std::shared_lock<std::shared_mutex> lock(m_user_permission_map_mutex);

    std::vector<UserID> return_vector;
    std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
        [&return_vector](const std::pair<UserID, PermissionType>& p) {
            if (p.second == PermissionType::Default)
                return_vector.push_back(p.first);
        });

    return return_vector;
}

std::vector<UserID> GroupPermission::getOperatorList() const
{
    std::shared_lock<std::shared_mutex> lock(m_user_permission_map_mutex);

    std::vector<UserID> return_vector;
    std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
        [&return_vector](const std::pair<UserID, PermissionType>& p) {
            if (p.second == PermissionType::Operator)
                return_vector.push_back(p.first);
        });

    return return_vector;
}

std::vector<UserID> GroupPermission::getAdministratorList() const
{
    std::shared_lock<std::shared_mutex> lock(m_user_permission_map_mutex);

    std::vector<UserID> return_vector;
    std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
        [&return_vector](const std::pair<UserID, PermissionType>& p) {
            if (p.second == PermissionType::Administrator)
                return_vector.push_back(p.first);
        });

    return return_vector;
}

} // namespace qls
