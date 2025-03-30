#ifndef GROUP_PERMISSION_H
#define GROUP_PERMISSION_H

#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <vector>

#include "definition.hpp"
#include "userid.hpp"

namespace qls
{

/**
 * @brief Enum defining different types of permissions.
 */
enum class PermissionType
{
    Default = 0, ///< Default permission level
    Operator,    ///< Operator permission level
    Administrator ///< Administrator permission level
};

/**
 * @brief Class representing group permissions.
 */
class GroupPermission final
{
public:
    GroupPermission() = default;
    ~GroupPermission() noexcept = default;

    /**
     * @brief Modifies the permission type for a specific permission.
     * @param permissionName Name of the permission to modify.
     * @param type New permission type.
     */
    void modifyPermission(std::string_view permissionName, PermissionType type = PermissionType::Default);

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
    PermissionType getPermissionType(std::string_view permissionName) const;

    /**
     * @brief Retrieves the entire permission list.
     * @return std::unordered_map<std::string, PermissionType> Map of permissions and their types.
     */
    std::unordered_map<std::string, PermissionType, string_hash, std::equal_to<>> getPermissionList() const;

    /**
     * @brief Modifies the permission type for a specific user.
     * @param user_id ID of the user.
     * @param type New permission type.
     */
    void modifyUserPermission(UserID user_id, PermissionType type = PermissionType::Default);

    /**
     * @brief Removes a user from the user permission list.
     * @param user_id ID of the user to remove.
     */
    void removeUser(UserID user_id);

    /**
     * @brief Checks if a user has a specific permission.
     * @param user_id ID of the user.
     * @param permissionName Name of the permission.
     * @return true if user has the permission, false otherwise.
     */
    bool userHasPermission(UserID user_id, std::string_view permissionName) const;

    /**
     * @brief Retrieves the permission type for a specific user.
     * @param user_id ID of the user.
     * @return PermissionType Type of permission for the user.
     */
    PermissionType getUserPermissionType(UserID user_id) const;

    /**
     * @brief Retrieves the entire user permission list.
     * @return std::unordered_map<UserID, PermissionType> Map of users and their permission types.
     */
    std::unordered_map<UserID, PermissionType> getUserPermissionList() const;

    /**
     * @brief Retrieves a list of users with default permissions.
     * @return std::vector<UserID> List of user IDs.
     */
    std::vector<UserID> getDefaultUserList() const;

    /**
     * @brief Retrieves a list of users with operator permissions.
     * @return std::vector<UserID> List of user IDs.
     */
    std::vector<UserID> getOperatorList() const;

    /**
     * @brief Retrieves a list of users with administrator permissions.
     * @return std::vector<UserID> List of user IDs.
     */
    std::vector<UserID> getAdministratorList() const;

private:
    std::unordered_map<std::string, PermissionType, string_hash, std::equal_to<>>
                                m_permission_map; ///< Map of permissions and their types
    mutable std::shared_mutex   m_permission_map_mutex; ///< Mutex for thread-safe access to permission map

    std::unordered_map<UserID, PermissionType>
                                m_user_permission_map; ///< Map of users and their permission types
    mutable std::shared_mutex   m_user_permission_map_mutex; ///< Mutex for thread-safe access to user permission map
};

} // namespace qls

#endif // !GROUP_PERMISSION_H
