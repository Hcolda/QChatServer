#ifndef GROUP_PERMISSION_H
#define GROUP_PERMISSION_H

#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>

#include "definition.hpp"
#include "userid.hpp"

namespace qls {

/**
 * @brief Enum defining different types of permissions.
 */
enum class PermissionType : std::int8_t {
  Default = 0,  ///< Default permission level
  Operator,     ///< Operator permission level
  Administrator ///< Administrator permission level
};

/**
 * @brief Class representing group permissions.
 */
class GroupPermission final {
public:
  GroupPermission(std::pmr::memory_resource *memory_resource);
  ~GroupPermission() noexcept;

  /**
   * @brief Modifies the permission type for a specific permission.
   * @param permissionName Name of the permission to modify.
   * @param type New permission type.
   */
  void modifyPermission(std::string_view permissionName,
                        PermissionType type = PermissionType::Default);

  /**
   * @brief Removes a permission from the permission list.
   * @param permissionName Name of the permission to remove.
   */
  void removePermission(std::string_view permissionName);

  /**
   * @brief Retrieves the type of a specific permission.
   * @param permissionName Name of the permission.
   * @return PermissionType Type of the permission.
   */
  [[nodiscard]] PermissionType
  getPermissionType(std::string_view permissionName) const;

  /**
   * @brief Retrieves the entire permission list.
   * @return std::unordered_map<std::string, PermissionType> Map of permissions
   * and their types.
   */
  void getPermissionList(
      const std::function<void(
          const std::pmr::unordered_map<std::string, PermissionType,
                                        string_hash, std::equal_to<>> &)> &func)
      const;

  /**
   * @brief Modifies the permission type for a specific user.
   * @param user_id ID of the user.
   * @param type New permission type.
   */
  void modifyUserPermission(const UserID &user_id,
                            PermissionType type = PermissionType::Default);

  /**
   * @brief Removes a user from the user permission list.
   * @param user_id ID of the user to remove.
   */
  void removeUser(const UserID &user_id);

  /**
   * @brief Checks if a user has a specific permission.
   * @param user_id ID of the user.
   * @param permissionName Name of the permission.
   * @return true if user has the permission, false otherwise.
   */
  [[nodiscard]] bool userHasPermission(const UserID &user_id,
                                       std::string_view permissionName) const;

  /**
   * @brief Retrieves the permission type for a specific user.
   * @param user_id ID of the user.
   * @return PermissionType Type of permission for the user.
   */
  [[nodiscard]] PermissionType
  getUserPermissionType(const UserID &user_id) const;

  /**
   * @brief Retrieves the entire user permission list.
   * @return std::unordered_map<UserID, PermissionType> Map of users and their
   * permission types.
   */
  void getUserPermissionList(
      const std::function<
          void(const std::pmr::unordered_map<UserID, PermissionType> &)> &func)
      const;

private:
  struct GroupPermissionImpl;
  struct GroupPermissionImplDeleter {
    std::pmr::memory_resource *memory_resource;
    void operator()(GroupPermissionImpl *);
  };
  std::unique_ptr<GroupPermissionImpl, GroupPermissionImplDeleter> m_impl;
};

} // namespace qls

#endif // !GROUP_PERMISSION_H
