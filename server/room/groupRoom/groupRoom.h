#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include <asio.hpp>
#include <chrono>
#include <memory_resource>
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
    UserLevel<> level;
  };

  GroupRoom(const GroupID &group_id, const UserID &administrator,
            bool is_create, std::pmr::memory_resource *memory_resouce);
  GroupRoom(const GroupRoom &) = delete;
  GroupRoom(GroupRoom &&) = delete;
  ~GroupRoom() noexcept;

  [[nodiscard]] bool addMember(const UserID &user_id);
  [[nodiscard]] bool hasMember(const UserID &user_id) const;
  [[nodiscard]] bool removeMember(const UserID &user_id);

  void sendMessage(const UserID &sender_user_id, std::string_view message);
  void sendTipMessage(const UserID &sender_user_id, std::string_view message);
  void sendUserTipMessage(const UserID &sender_user_id, std::string_view,
                          const UserID &receiver_user_id);
  [[nodiscard]] std::vector<MessageResult>
  getMessage(const std::chrono::utc_clock::time_point &from,
             const std::chrono::utc_clock::time_point &to);

  [[nodiscard]] bool hasUser(const UserID &user_id) const;
  void
  getUserList(const std::function<
              void(const std::pmr::unordered_map<UserID, UserDataStructure> &)>
                  &func) const;
  [[nodiscard]] std::string getUserNickname(const UserID &user_id) const;
  [[nodiscard]] long long getUserGroupLevel(const UserID &user_id) const;
  void getUserPermissionList(
      const std::function<
          void(const std::pmr::unordered_map<UserID, PermissionType> &)> &func)
      const;
  [[nodiscard]] UserID getAdministrator() const;
  [[nodiscard]] GroupID getGroupID() const;

  [[nodiscard]] bool muteUser(const UserID &executor_id, const UserID &user_id,
                              const std::chrono::minutes &mins);
  [[nodiscard]] bool unmuteUser(const UserID &executor_id,
                                const UserID &user_id);
  [[nodiscard]] bool kickUser(const UserID &executor_id, const UserID &user_id);
  [[nodiscard]] bool addOperator(const UserID &executor_id,
                                 const UserID &user_id);
  [[nodiscard]] bool removeOperator(const UserID &executor_id,
                                    const UserID &user_id);
  void setAdministrator(const UserID &user_id);

  void removeThisRoom();
  [[nodiscard]] bool canBeUsed() const;

  asio::awaitable<void> auto_clean();
  void stop_cleaning();

private:
  std::unique_ptr<GroupRoomImpl, GroupRoomImplDeleter> m_impl;
};

} // namespace qls

#endif // !GROUP_ROOM_H
