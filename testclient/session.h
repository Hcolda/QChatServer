#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <string>
#include <string_view>

#include "network.h"
#include "option.hpp"
#include "string_param.hpp"
#include <groupid.hpp>
#include <userid.hpp>

namespace qls {

struct SessionImpl;
class Session final {
public:
  Session(Network &network);
  ~Session() noexcept;

  bool registerUser(string_param email, string_param password,
                    UserID &newUserID);
  bool loginUser(UserID user_id, string_param password);

  bool createFriendApplication(UserID userid);
  bool applyFriendApplication(UserID userid);
  bool rejectFriendApplication(UserID userid);

  bool createGroup();
  bool createGroupApplication(GroupID groupid);
  bool applyGroupApplication(GroupID groupid, UserID userid);
  bool rejectGroupApplication(GroupID groupid, UserID userid);

  bool sendFriendMessage(UserID userid, string_param message);
  bool sendGroupMessage(GroupID groupid, string_param message);

  bool removeFriend(UserID userid);
  bool leaveGroup(GroupID groupid);

private:
  std::unique_ptr<SessionImpl> m_impl;
};

} // namespace qls

#endif
