#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <string>
#include <string_view>

#include "network.h"
#include "option.hpp"

#include <groupid.hpp>
#include <userid.hpp>

namespace qls {

struct SessionImpl;
class Session final {
public:
  Session(Network &network);
  ~Session() noexcept;

  bool registerUser(std::string_view email, std::string_view password,
                    UserID &newUserID);
  bool loginUser(UserID user_id, std::string_view password);

  bool createFriendApplication(UserID userid);
  bool applyFriendApplication(UserID userid);
  bool rejectFriendApplication(UserID userid);

  bool createGroup();
  bool createGroupApplication(GroupID groupid);
  bool applyGroupApplication(GroupID groupid, UserID userid);
  bool rejectGroupApplication(GroupID groupid, UserID userid);

  bool sendFriendMessage(UserID userid, std::string_view message);
  bool sendGroupMessage(GroupID groupid, std::string_view message);

  bool removeFriend(UserID userid);
  bool leaveGroup(GroupID groupid);

private:
  std::unique_ptr<SessionImpl> m_impl;
};

} // namespace qls

#endif
