#ifndef JSON_MESSAGE_PROCESS_COMMAND_H
#define JSON_MESSAGE_PROCESS_COMMAND_H

#include <Json.h>
#include <cstdint>
#include <initializer_list>
#include <string>

#include "userid.hpp"

namespace qls {

class JsonMessageCommand {
public:
  enum CommandType : std::int8_t {
    NormalType = 0, // Use it if the function do not need to login.
    LoginType = 1   // Use it if the function need to login.
  };

  struct JsonOption {
    std::string name;
    qjson::JValueType jsonValueType;
  };

  JsonMessageCommand() = default;
  virtual ~JsonMessageCommand() = default;

  virtual const std::vector<JsonOption> &getOption() const = 0;
  virtual int getCommandType() const = 0;
  virtual qjson::JObject execute(UserID executor,
                                 qjson::JObject parameters) = 0;
};

class RegisterCommand : public JsonMessageCommand {
public:
  RegisterCommand() = default;
  ~RegisterCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"email", qjson::JString},
                                          {"password", qjson::JString}};
    return vec;
  }

  int getCommandType() const { return NormalType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class HasUserCommand : public JsonMessageCommand {
public:
  HasUserCommand() = default;
  ~HasUserCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return NormalType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SearchUserCommand : public JsonMessageCommand {
public:
  SearchUserCommand() = default;
  ~SearchUserCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return NormalType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AddFriendCommand : public JsonMessageCommand {
public:
  AddFriendCommand() = default;
  ~AddFriendCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AcceptFriendVerificationCommand : public JsonMessageCommand {
public:
  AcceptFriendVerificationCommand() = default;
  ~AcceptFriendVerificationCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RejectFriendVerificationCommand : public JsonMessageCommand {
public:
  RejectFriendVerificationCommand() = default;
  ~RejectFriendVerificationCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetFriendListCommand : public JsonMessageCommand {
public:
  GetFriendListCommand() = default;
  ~GetFriendListCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec;
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetFriendVerificationListCommand : public JsonMessageCommand {
public:
  GetFriendVerificationListCommand() = default;
  ~GetFriendVerificationListCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec;
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RemoveFriendCommand : public JsonMessageCommand {
public:
  RemoveFriendCommand() = default;
  ~RemoveFriendCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AddGroupCommand : public JsonMessageCommand {
public:
  AddGroupCommand() = default;
  ~AddGroupCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AcceptGroupVerificationCommand : public JsonMessageCommand {
public:
  AcceptGroupVerificationCommand() = default;
  ~AcceptGroupVerificationCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt},
                                          {"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RejectGroupVerificationCommand : public JsonMessageCommand {
public:
  RejectGroupVerificationCommand() = default;
  ~RejectGroupVerificationCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt},
                                          {"user_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetGroupListCommand : public JsonMessageCommand {
public:
  GetGroupListCommand() = default;
  ~GetGroupListCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec;
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetGroupVerificationListCommand : public JsonMessageCommand {
public:
  GetGroupVerificationListCommand() = default;
  ~GetGroupVerificationListCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec;
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class CreateGroupCommand : public JsonMessageCommand {
public:
  CreateGroupCommand() = default;
  ~CreateGroupCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec;
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RemoveGroupCommand : public JsonMessageCommand {
public:
  RemoveGroupCommand() = default;
  ~RemoveGroupCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class LeaveGroupCommand : public JsonMessageCommand {
public:
  LeaveGroupCommand() = default;
  ~LeaveGroupCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SendFriendMessageCommand : public JsonMessageCommand {
public:
  SendFriendMessageCommand() = default;
  ~SendFriendMessageCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"friend_id", qjson::JInt},
                                          {"message", qjson::JString}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SendGroupMessageCommand : public JsonMessageCommand {
public:
  SendGroupMessageCommand() = default;
  ~SendGroupMessageCommand() = default;

  const std::vector<JsonOption> &getOption() const {
    static std::vector<JsonOption> vec = {{"group_id", qjson::JInt},
                                          {"message", qjson::JString}};
    return vec;
  }

  int getCommandType() const { return LoginType; }

  qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

} // namespace qls

#endif // !JSON_MESSAGE_PROCESS_COMMAND_H
