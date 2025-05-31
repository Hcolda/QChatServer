#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <unordered_map>

#include "SQLProcess.hpp"
#include "connection.hpp"
#include "dataManager.h"
#include "definition.hpp"
#include "groupRoom.h"
#include "groupid.hpp"
#include "network.h"
#include "privateRoom.h"
#include "user.h"
#include "userid.hpp"
#include "verificationManager.h"

namespace qls {

struct ManagerImpl;

/**
 * @class Manager
 * @brief Manages the core functionalities such as user, private room, and group
 * room management.
 */
class Manager final {
public:
  Manager();
  Manager(const Manager &) = delete;
  Manager(Manager &&) = delete;
  ~Manager();

  Manager &operator=(const Manager &) = delete;
  Manager &operator=(Manager &&) = delete;

  /**
   * @brief Initializes the manager.
   */
  void init();

  /**
   * @brief Adds a private room between two users.
   *
   * @param user1_id ID of the first user.
   * @param user2_id ID of the second user.
   * @return The ID of the created private room.
   */
  [[nodiscard]] GroupID addPrivateRoom(UserID user1_id, UserID user2_id);

  /**
   * @brief Retrieves the private room ID between two users.
   *
   * @param user1_id ID of the first user.
   * @param user2_id ID of the second user.
   * @return The ID of the private room.
   */
  [[nodiscard]] GroupID getPrivateRoomId(UserID user1_id,
                                         UserID user2_id) const;

  /**
   * @brief Checks if a private room exists.
   *
   * @param private_room_id The ID of the private room.
   * @return True if the private room exists, false otherwise.
   */
  [[nodiscard]] bool hasPrivateRoom(GroupID private_room_id) const;

  [[nodiscard]] bool hasPrivateRoom(UserID user1_id, UserID user2_id) const;

  /**
   * @brief Retrieves a private room.
   *
   * @param private_room_id The ID of the private room.
   * @return Shared pointer to the private room.
   */
  [[nodiscard]] std::shared_ptr<qls::PrivateRoom>
  getPrivateRoom(GroupID private_room_id) const;

  /**
   * @brief Removes a private room.
   * @param private_room_id The ID of the private room.
   */
  void removePrivateRoom(GroupID private_room_id);

  /**
   * @brief Adds a group room.
   *
   * @param operator_user_id ID of the user creating the group room.
   * @return The ID of the created group room.
   */
  [[nodiscard]] GroupID addGroupRoom(UserID operator_user_id);

  /**
   * @brief Checks if a group room exists.
   *
   * @param group_room_id The ID of the group room.
   * @return True if the group room exists, false otherwise.
   */
  [[nodiscard]] bool hasGroupRoom(GroupID group_room_id) const;

  /**
   * @brief Retrieves a group room.
   *
   * @param group_room_id The ID of the group room.
   * @return Shared pointer to the group room.
   */
  [[nodiscard]] std::shared_ptr<qls::GroupRoom>
  getGroupRoom(GroupID group_room_id) const;

  /**
   * @brief Removes a group room.
   *
   * @param group_room_id The ID of the group room.
   */
  void removeGroupRoom(GroupID group_room_id);

  /**
   * @brief Adds a new user.
   *
   * @return Shared pointer to the new user.
   */
  [[nodiscard]] std::shared_ptr<qls::User> addNewUser();

  /**
   * @brief Checks if a user exists.
   *
   * @param user_id The ID of the user.
   * @return True if the user exists, false otherwise.
   */
  [[nodiscard]] bool hasUser(UserID user_id) const;

  /**
   * @brief Retrieves a user.
   *
   * @param user_id The ID of the user.
   * @return Shared pointer to the user.
   */
  [[nodiscard]] std::shared_ptr<qls::User> getUser(UserID user_id) const;

  /**
   * @brief Retrieves the list of users.
   *
   * @return Unordered map of user IDs to user shared pointers.
   */
  [[nodiscard]] std::unordered_map<UserID, std::shared_ptr<qls::User>>
  getUserList() const;

  /**
   * @brief Registers a socket with an optional user ID.
   *
   * @param socket_ptr A shared pointer to the socket to register.
   */
  void registerConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr);

  /**
   * @brief Checks if a socket is registered.
   *
   * @param socket_ptr A shared pointer to the socket to check.
   * @return true if the socket is registered, false otherwise.
   */
  [[nodiscard]] bool hasConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr)
      const;

  /**
   * @brief Checks if a socket is associated with a specific user ID.
   *
   * @param socket_ptr A shared pointer to the Connection object.
   * @param user_id The user ID to check against the socket.
   * @return true if the socket is associated with the specified user ID, false
   * otherwise.
   */
  [[nodiscard]] bool matchUserOfConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr,
      UserID user_id) const;

  /**
   * @brief Gets the user ID associated with a socket.
   *
   * @param socket_ptr A shared pointer to the Connection object.
   * @return The user ID associated with the socket.
   */
  [[nodiscard]] UserID getUserIDOfConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr)
      const;

  /**
   * @brief Modifies the user ID associated with a registered socket.
   *
   * @param socket_ptr A shared pointer to the socket whose user ID is to be
   * modified.
   * @param user_id The user ID to associate with the socket.
   * @param type The type of device associated with the socket.
   */
  void modifyUserOfConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr,
      UserID user_id, DeviceType type);

  /**
   * @brief Removes a registered socket.
   *
   * @param socket_ptr A shared pointer to the socket to remove.
   */
  void removeConnection(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &socket_ptr);

  /**
   * @brief Retrieves the SQL process for the server.
   * @return Reference to the SQLDBProcess.
   */
  [[nodiscard]] qls::SQLDBProcess &getServerSqlProcess();

  /**
   * @brief Retrieves the data manager for the server.
   * @return Reference to the DataManager.
   */
  [[nodiscard]] qls::DataManager &getServerDataManager();

  /**
   * @brief Retrieves the verification manager for the server.
   * @return Reference to the VerificationManager.
   */
  [[nodiscard]] qls::VerificationManager &getServerVerificationManager();

  /**
   * @brief Retrieves the network for the server.
   * @return Reference to the Network.
   */
  [[nodiscard]] qls::Network &getServerNetwork();

private:
  std::unique_ptr<ManagerImpl> m_impl;
};

} // namespace qls

#endif // !MANAGER_H
