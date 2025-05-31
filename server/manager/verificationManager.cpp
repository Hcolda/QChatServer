#include "verificationManager.h"

#include <functional>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "groupid.hpp"
#include "manager.h"
#include "user.h"
#include "userid.hpp"

// manager
extern qls::Manager serverManager;

namespace qls {

struct FriendVerification {
  const UserID &applicator;
  const UserID &controller;
};

struct GroupVerification {
  const UserID &applicator;
  const GroupID &controller;
};

} // namespace qls

namespace std {

template <> struct hash<qls::FriendVerification> {
public:
  std::size_t
  operator()(const qls::FriendVerification &friend_verification) const {
    hash<long long> local_hash{};
    return local_hash(friend_verification.applicator.getOriginValue()) ^
           local_hash(friend_verification.controller.getOriginValue());
  }
};

template <> struct equal_to<qls::FriendVerification> {
public:
  bool operator()(const qls::FriendVerification &friend_verification1,
                  const qls::FriendVerification &friend_verification2) const {
    return friend_verification1.applicator == friend_verification2.applicator &&
           friend_verification1.controller == friend_verification2.controller;
  }
};

template <> struct hash<qls::GroupVerification> {
public:
  std::size_t
  operator()(const qls::GroupVerification &group_verification) const {
    hash<long long> local_hash{};
    return local_hash(group_verification.applicator.getOriginValue()) ^
           local_hash(group_verification.controller.getOriginValue());
  }
};

template <> struct equal_to<qls::GroupVerification> {
public:
  bool operator()(const qls::GroupVerification &group_verification1,
                  const qls::GroupVerification &group_verification2) const {
    return group_verification1.applicator == group_verification2.applicator &&
           group_verification1.controller == group_verification2.controller;
  }
};

} // namespace std

namespace qls {

struct VerificationManager::VerificationManagerImpl {
  /**
   * @brief Map of friend room verification requests.
   */
  std::unordered_map<FriendVerification, bool> m_friendRoomVerification_map;
  std::shared_mutex m_friendRoomVerification_map_mutex;

  /**
   * @brief Map of group room verification requests.
   */
  std::unordered_map<GroupVerification, bool> m_groupVerification_map;
  std::shared_mutex m_groupVerification_map_mutex;
};

VerificationManager::VerificationManager()
    : m_impl(std::make_unique<VerificationManagerImpl>()) {}

VerificationManager::~VerificationManager() noexcept = default;

void VerificationManager::init() {
  // sql init
}

void VerificationManager::applyFriendRoomVerification(const UserID &sender,
                                                      const UserID &receiver) {
  if (sender == receiver) {
    throw std::system_error(qls_errc::invalid_verification);
  }
  if (!serverManager.hasUser(sender)) {
    throw std::system_error(qls_errc::user_not_existed,
                            "the id of sender is invalid");
  }
  if (!serverManager.hasUser(receiver)) {
    throw std::system_error(qls_errc::user_not_existed,
                            "the id of receiver is invalid");
  }

  // check if they are friends
  if (!serverManager.hasPrivateRoom(sender, receiver)) {
    throw std::system_error(qls_errc::private_room_existed);
  }

  {
    std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

    if (m_impl->m_friendRoomVerification_map.find({sender, receiver}) !=
        m_impl->m_friendRoomVerification_map.cend()) {
      throw std::system_error(qls_errc::verification_existed);
    }
    m_impl->m_friendRoomVerification_map.emplace(
        FriendVerification{sender, receiver}, false);
  }

  // sender
  {
    qls::Verification::UserVerification sender_uv;

    sender_uv.user_id = receiver;
    sender_uv.verification_type = qls::Verification::VerificationType::Sent;

    auto ptr = serverManager.getUser(sender);
    ptr->addFriendVerification(receiver, sender_uv);
  }

  // receiver
  {
    qls::Verification::UserVerification receiver_uv;

    receiver_uv.user_id = sender;
    receiver_uv.verification_type =
        qls::Verification::VerificationType::Received;

    auto ptr = serverManager.getUser(receiver);
    ptr->addFriendVerification(sender, receiver_uv);
  }
}

bool VerificationManager::hasFriendRoomVerification(
    const UserID &sender, const UserID &receiver) const {
  if (sender == receiver) {
    return false;
  }

  std::shared_lock lock(m_impl->m_friendRoomVerification_map_mutex);
  return m_impl->m_friendRoomVerification_map.find({sender, receiver}) !=
         m_impl->m_friendRoomVerification_map.cend();
}

void VerificationManager::acceptFriendVerification(const UserID &sender,
                                                   const UserID &receiver) {
  if (sender == receiver) {
    throw std::system_error(qls_errc::invalid_verification);
  }

  {
    std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

    auto iter = m_impl->m_friendRoomVerification_map.find({sender, receiver});
    if (iter == m_impl->m_friendRoomVerification_map.cend()) {
      throw std::system_error(qls_errc::verification_not_existed);
    }

    iter->second = true;
  }

  GroupID group_id = serverManager.addPrivateRoom(sender, receiver);
  // update the 1st user's friend list
  {
    auto ptr = serverManager.getUser(sender);
    ptr->updateFriendList([receiver](std::unordered_set<qls::UserID> &set) {
      set.insert(receiver);
    });
  }

  // update the 2nd user's friend list
  {
    auto ptr = serverManager.getUser(receiver);
    ptr->updateFriendList(
        [sender](std::unordered_set<qls::UserID> &set) { set.insert(sender); });
  }

  this->removeFriendRoomVerification(sender, receiver);
}

void VerificationManager::rejectFriendVerification(const UserID &sender,
                                                   const UserID &receiver) {
  if (sender == receiver) {
    throw std::system_error(qls_errc::invalid_verification);
  }

  this->removeFriendRoomVerification(sender, receiver);
}

bool VerificationManager::isFriendVerified(const UserID &sender,
                                           const UserID &receiver) const {
  if (sender == receiver) {
    throw std::system_error(qls_errc::invalid_verification);
  }

  std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

  auto iter = m_impl->m_friendRoomVerification_map.find({sender, receiver});
  if (iter == m_impl->m_friendRoomVerification_map.cend()) {
    throw std::system_error(qls_errc::verification_not_existed);
  }

  return iter->second;
}

void VerificationManager::removeFriendRoomVerification(const UserID &sender,
                                                       const UserID &receiver) {
  if (sender == receiver) {
    throw std::system_error(qls_errc::invalid_verification);
  }

  {
    std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

    auto iter = m_impl->m_friendRoomVerification_map.find({sender, receiver});
    if (iter == m_impl->m_friendRoomVerification_map.cend()) {
      return;
    }

    m_impl->m_friendRoomVerification_map.erase(iter);
  }

  serverManager.getUser(sender)->removeFriendVerification(receiver);
  serverManager.getUser(receiver)->removeFriendVerification(sender);
}

void VerificationManager::applyGroupRoomVerification(const UserID &sender,
                                                     const GroupID &receiver) {
  if (!serverManager.hasGroupRoom(receiver)) {
    throw std::system_error(qls_errc::group_room_not_existed);
  }
  if (!serverManager.hasUser(sender)) {
    throw std::system_error(qls_errc::user_not_existed);
  }

  {
    std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

    if (m_impl->m_groupVerification_map.find({sender, receiver}) !=
        m_impl->m_groupVerification_map.cend()) {
      throw std::system_error(qls_errc::verification_not_existed);
    }

    m_impl->m_groupVerification_map.emplace(GroupVerification{sender, receiver},
                                            false);
  }

  {
    qls::Verification::GroupVerification sender_uv;

    sender_uv.group_id = receiver;
    sender_uv.user_id = sender;
    sender_uv.verification_type = qls::Verification::Sent;

    auto ptr = serverManager.getUser(sender);
    ptr->addGroupVerification(receiver, sender_uv);
  }

  // Only modify the administrator's verification list
  // Because of over consumption of computer power
  {
    qls::Verification::GroupVerification receiver_uv;

    receiver_uv.group_id = receiver;
    receiver_uv.user_id = sender;
    receiver_uv.verification_type = qls::Verification::Received;

    UserID adminID = serverManager.getGroupRoom(receiver)->getAdministrator();
    auto ptr = serverManager.getUser(adminID);
    ptr->addGroupVerification(receiver, receiver_uv);
  }
}

bool VerificationManager::hasGroupRoomVerification(
    const UserID &sender, const GroupID &receiver) const {
  std::shared_lock lock(m_impl->m_groupVerification_map_mutex);

  return m_impl->m_groupVerification_map.find({sender, receiver}) !=
         m_impl->m_groupVerification_map.cend();
}

void VerificationManager::acceptGroupRoom(const UserID &sender,
                                          const GroupID &receiver) {
  {
    std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

    auto iter = m_impl->m_groupVerification_map.find({sender, receiver});
    if (iter == m_impl->m_groupVerification_map.cend()) {
      throw std::system_error(qls_errc::verification_not_existed);
    }

    iter->second = true;
  }

  bool is_added = serverManager.getGroupRoom(receiver)->addMember(sender);
  // update user's list
  auto ptr = serverManager.getUser(sender);
  ptr->updateGroupList([receiver](std::unordered_set<qls::GroupID> &set) {
    set.insert(receiver);
  });
  this->removeGroupRoomVerification(sender, receiver);
}

void VerificationManager::rejectGroupRoom(const UserID &sender,
                                          const GroupID &receiver) {
  this->removeGroupRoomVerification(sender, receiver);
}

bool VerificationManager::isGroupRoomVerified(const UserID &sender,
                                              const GroupID &receiver) const {
  std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

  auto iter = m_impl->m_groupVerification_map.find({sender, receiver});
  if (iter == m_impl->m_groupVerification_map.cend()) {
    throw std::system_error(qls_errc::verification_not_existed);
  }

  return iter->second;
}

void VerificationManager::removeGroupRoomVerification(const UserID &sender,
                                                      const GroupID &receiver) {
  {
    std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

    auto iter = m_impl->m_groupVerification_map.find({sender, receiver});
    if (iter == m_impl->m_groupVerification_map.cend()) {
      throw std::system_error(qls_errc::verification_not_existed);
    }

    m_impl->m_groupVerification_map.erase(iter);
  }
  const UserID &adminID =
      serverManager.getGroupRoom(receiver)->getAdministrator();
  serverManager.getUser(adminID)->removeGroupVerification(receiver, sender);
  serverManager.getUser(sender)->removeGroupVerification(receiver, sender);
}

} // namespace qls
