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

  void applyFriendRoomVerification(const UserID &sender,
                                   const UserID &receiver);
  [[nodiscard]] bool hasFriendRoomVerification(const UserID &sender,
                                               const UserID &receiver) const;
  void acceptFriendVerification(const UserID &sender, const UserID &receiver);
  void rejectFriendVerification(const UserID &sender, const UserID &receiver);
  [[nodiscard]] bool isFriendVerified(const UserID &sender,
                                      const UserID &receiver) const;
  void removeFriendRoomVerification(const UserID &sender,
                                    const UserID &receiver);

  void applyGroupRoomVerification(const UserID &sender,
                                  const GroupID &receiver);
  [[nodiscard]] bool hasGroupRoomVerification(const UserID &sender,
                                              const GroupID &receiver) const;
  void acceptGroupRoom(const UserID &sender, const GroupID &receiver);
  void rejectGroupRoom(const UserID &sender, const GroupID &receiver);
  [[nodiscard]] bool isGroupRoomVerified(const UserID &sender,
                                         const GroupID &receiver) const;
  void removeGroupRoomVerification(const UserID &sender,
                                   const GroupID &receiver);

private:
  struct VerificationManagerImpl;
  std::unique_ptr<VerificationManagerImpl> m_impl;
};

} // namespace qls

#endif // !VERIFICATION_MANAGER_H
