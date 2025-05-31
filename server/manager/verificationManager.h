#ifndef VERIFICATION_MANAGER_H
#define VERIFICATION_MANAGER_H

#include <Json.h>
#include <memory>

#include "groupid.hpp"
#include "userid.hpp"

namespace qls {

/**
 * @class VerificationManager
 * @brief Manages verifications for friend and group room requests.
 */
class VerificationManager final {
public:
  VerificationManager();
  VerificationManager(const VerificationManager &) = delete;
  VerificationManager(VerificationManager &&) = delete;
  ~VerificationManager() noexcept;

  VerificationManager &operator=(const VerificationManager &) = delete;
  VerificationManager &operator=(VerificationManager &&) = delete;

  /**
   * @brief Initializes the verification manager.
   */
  void init();

  void applyFriendRoomVerification(UserID sender, UserID receiver);
  [[nodiscard]] bool hasFriendRoomVerification(UserID sender,
                                               UserID receiver) const;
  void acceptFriendVerification(UserID sender, UserID receiver);
  void rejectFriendVerification(UserID sender, UserID receiver);
  [[nodiscard]] bool isFriendVerified(UserID sender, UserID receiver) const;
  void removeFriendRoomVerification(UserID sender, UserID receiver);

  void applyGroupRoomVerification(UserID sender, GroupID receiver);
  [[nodiscard]] bool hasGroupRoomVerification(UserID sender,
                                              GroupID receiver) const;
  void acceptGroupRoom(UserID sender, GroupID receiver);
  void rejectGroupRoom(UserID sender, GroupID receiver);
  [[nodiscard]] bool isGroupRoomVerified(UserID sender, GroupID receiver) const;
  void removeGroupRoomVerification(UserID sender, GroupID receiver);

private:
  struct VerificationManagerImpl;
  std::unique_ptr<VerificationManagerImpl> m_impl;
};

} // namespace qls

#endif // !VERIFICATION_MANAGER_H
