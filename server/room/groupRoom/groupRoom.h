#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include <asio.hpp>
#include <chrono>
#include <string_view>
#include <vector>

#include "groupPermission.h"
#include "groupUserLevel.hpp"
#include "groupid.hpp"
#include "room.h"
#include "userid.hpp"

namespace qls {

struct GroupRoomImpl;
struct GroupRoomImplDeleter {
  void operator()(GroupRoomImpl *gri);
};

class GroupRoom : public TextDataRoom {
public:
  struct UserDataStructure {
    std::string nickname;
    UserLevel<1, 100> level;
  };

  GroupRoom(GroupID group_id, UserID administrator, bool is_create);
  GroupRoom(const GroupRoom &) = delete;
  GroupRoom(GroupRoom &&) = delete;
  ~GroupRoom() noexcept;

  [[nodiscard]] bool addMember(UserID user_id);
  [[nodiscard]] bool hasMember(UserID user_id) const;
  [[nodiscard]] bool removeMember(UserID user_id);

  void sendMessage(UserID sender_user_id, std::string_view message);
  void sendTipMessage(UserID sender_user_id, std::string_view message);
  void sendUserTipMessage(UserID sender_user_id, std::string_view,
                          UserID receiver_user_id);
  [[nodiscard]] std::vector<MessageResult>
  getMessage(const std::chrono::utc_clock::time_point &from,
             const std::chrono::utc_clock::time_point &to);

  [[nodiscard]] bool hasUser(UserID user_id) const;
  [[nodiscard]] std::unordered_map<UserID, UserDataStructure>
  getUserList() const;
  [[nodiscard]] std::string getUserNickname(UserID user_id) const;
  [[nodiscard]] long long getUserGroupLevel(UserID user_id) const;
  [[nodiscard]] std::unordered_map<UserID, PermissionType>
  getUserPermissionList() const;
  [[nodiscard]] UserID getAdministrator() const;
  [[nodiscard]] GroupID getGroupID() const;
  [[nodiscard]] std::vector<UserID> getDefaultUserList() const;
  [[nodiscard]] std::vector<UserID> getOperatorList() const;

  [[nodiscard]] bool muteUser(UserID executor_id, UserID user_id,
                              const std::chrono::minutes &mins);
  [[nodiscard]] bool unmuteUser(UserID executor_id, UserID user_id);
  [[nodiscard]] bool kickUser(UserID executor_id, UserID user_id);
  [[nodiscard]] bool addOperator(UserID executor_id, UserID user_id);
  [[nodiscard]] bool removeOperator(UserID executor_id, UserID user_id);
  void setAdministrator(UserID user_id);

  void removeThisRoom();
  [[nodiscard]] bool canBeUsed() const;

  asio::awaitable<void> auto_clean();
  void stop_cleaning();

private:
  std::unique_ptr<GroupRoomImpl, GroupRoomImplDeleter> m_impl;
};

} // namespace qls

#endif // !GROUP_ROOM_H
