#include "JsonMsgProcess.h"

#include <format>

#include "JsonMsgProcessCommand.h"
#include "definition.hpp"
#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"

#include "userid.hpp"
#include <logger.hpp>
#include <string>
#include <string_view>

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls {

// -----------------------------------------------------------------------------------------------
// JsonMessageProcessCommandList
// -----------------------------------------------------------------------------------------------

class JsonMessageProcessCommandList {
public:
  JsonMessageProcessCommandList() {
    auto init_command =
        [&](std::string_view function_name,
            const std::shared_ptr<JsonMessageCommand> &command_ptr) -> bool {
      if (m_function_map.find(function_name) != m_function_map.cend() ||
          !command_ptr) {
        return false;
      }

      m_function_map.emplace(function_name, command_ptr);
      return true;
    };

    init_command("register", std::make_shared<RegisterCommand>());
    init_command("has_user", std::make_shared<HasUserCommand>());
    init_command("search_user", std::make_shared<SearchUserCommand>());
    init_command("add_friend", std::make_shared<AddFriendCommand>());
    init_command("add_group", std::make_shared<AddGroupCommand>());
    init_command("get_friend_list", std::make_shared<GetFriendListCommand>());
    init_command("get_group_list", std::make_shared<GetGroupListCommand>());
    init_command("send_friend_message",
                 std::make_shared<SendFriendMessageCommand>());
    init_command("send_group_message",
                 std::make_shared<SendGroupMessageCommand>());
    init_command("accept_friend_verification",
                 std::make_shared<AcceptFriendVerificationCommand>());
    init_command("get_friend_verification_list",
                 std::make_shared<GetFriendVerificationListCommand>());
    init_command("accept_group_verification",
                 std::make_shared<AcceptGroupVerificationCommand>());
    init_command("get_group_verification_list",
                 std::make_shared<GetGroupVerificationListCommand>());
    init_command("reject_friend_verification",
                 std::make_shared<RejectFriendVerificationCommand>());
    init_command("reject_group_verification",
                 std::make_shared<RejectGroupVerificationCommand>());
    init_command("create_group", std::make_shared<CreateGroupCommand>());
    init_command("remove_group", std::make_shared<RemoveGroupCommand>());
    init_command("leave_group", std::make_shared<LeaveGroupCommand>());
    init_command("remove_friend", std::make_shared<RemoveFriendCommand>());
  }
  ~JsonMessageProcessCommandList() = default;

  bool addCommand(std::string_view function_name,
                  const std::shared_ptr<JsonMessageCommand> &command_ptr);
  bool hasCommand(std::string_view function_name) const;
  std::shared_ptr<JsonMessageCommand>
  getCommand(std::string_view function_name);
  bool removeCommand(std::string_view function_name);

private:
  std::unordered_map<std::string, std::shared_ptr<JsonMessageCommand>,
                     string_hash, std::equal_to<>>
      m_function_map;
  mutable std::shared_mutex m_function_map_mutex;
};

bool JsonMessageProcessCommandList::addCommand(
    std::string_view function_name,
    const std::shared_ptr<JsonMessageCommand> &command_ptr) {
  if (hasCommand(function_name) || !command_ptr) {
    return false;
  }

  std::unique_lock unique_lock1(m_function_map_mutex);
  m_function_map.emplace(function_name, command_ptr);
  return true;
}

bool JsonMessageProcessCommandList::hasCommand(
    std::string_view function_name) const {
  std::shared_lock lock(m_function_map_mutex);
  return m_function_map.find(function_name) != m_function_map.cend();
}

std::shared_ptr<JsonMessageCommand>
JsonMessageProcessCommandList::getCommand(std::string_view function_name) {
  std::shared_lock lock(m_function_map_mutex);
  auto iter = m_function_map.find(function_name);
  if (iter == m_function_map.cend()) {
    throw std::system_error(make_error_code(qls_errc::null_pointer));
  }
  return iter->second;
}

bool JsonMessageProcessCommandList::removeCommand(
    std::string_view function_name) {
  if (!hasCommand(function_name)) {
    return false;
  }

  std::unique_lock unique_lock1(m_function_map_mutex);
  auto itor = m_function_map.find(function_name);
  if (itor == m_function_map.end()) {
    return false;
  }
  m_function_map.erase(itor);
  return true;
}

// -----------------------------------------------------------------------------------------------
// JsonMessageProcessImpl
// -----------------------------------------------------------------------------------------------

class JsonMessageProcessImpl {
public:
  JsonMessageProcessImpl(UserID user_id) : m_user_id(std::move(user_id)) {}

  static qjson::JObject getUserPublicInfo(const UserID &user_id);

  static qjson::JObject hasUser(const UserID &user_id);
  static qjson::JObject searchUser(std::string_view user_name);

  UserID getLocalUserID() const;

  asio::awaitable<qjson::JObject>
  processJsonMessage(const qjson::JObject &json,
                     const SocketService &socket_service);

  qjson::JObject login(const UserID &user_id, std::string_view password,
                       std::string_view device,
                       const SocketService &socket_service);

  static qjson::JObject login(std::string_view email, std::string_view password,
                              std::string_view device);

private:
  UserID m_user_id;
  mutable std::shared_mutex m_user_id_mutex;

  static JsonMessageProcessCommandList m_jmpc_list;
};

JsonMessageProcessCommandList JsonMessageProcessImpl::m_jmpc_list;

JsonMessageProcess::JsonMessageProcess(UserID user_id)
    : m_process(std::make_unique<JsonMessageProcessImpl>(std::move(user_id))) {}

JsonMessageProcess::~JsonMessageProcess() = default;

qjson::JObject
JsonMessageProcessImpl::getUserPublicInfo(const UserID &user_id) {
  // Return user's public information
  // No implementation for now
  return makeErrorMessage("This function is incomplete.");
}

qjson::JObject JsonMessageProcessImpl::hasUser(const UserID &user_id) {
  auto returnJson = makeSuccessMessage("Successfully getting result!");
  returnJson["result"] = serverManager.hasUser(user_id);
  return returnJson;
}

qjson::JObject JsonMessageProcessImpl::searchUser(std::string_view user_name) {
  return makeErrorMessage("This function is incomplete.");
}

UserID JsonMessageProcessImpl::getLocalUserID() const {
  std::shared_lock lock(m_user_id_mutex);
  return this->m_user_id;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::processJsonMessage(
    const qjson::JObject &json, const SocketService &socket_service) {
  try {
    // Check whether the json pack is valid
    serverLogger.debug("Json body: ", json.to_string());
    if (json.getType() != qjson::JDict) {
      co_return makeErrorMessage("The data body must be json dictory type!");
    } else if (!json.hasMember("function")) {
      co_return makeErrorMessage(
          "\"function\" must be included in json dictory!");
    } else if (!json.hasMember("parameters")) {
      co_return makeErrorMessage(
          "\"parameters\" must be included in json dictory!");
    } else if (json["function"].getType() != qjson::JString) {
      co_return makeErrorMessage("\"function\" must be string type!");
    } else if (json["parameters"].getType() != qjson::JDict) {
      co_return makeErrorMessage("\"parameters\" must be dictory type!");
    }
    std::string function_name = json["function"].getString();
    qjson::JObject param = json["parameters"];

    // Check if user has logined
    {
      std::shared_lock shared_lock1(m_user_id_mutex);
      // Check if userid == -1
      if (m_user_id == UserID(-1) && function_name != "login" &&
          (!m_jmpc_list.hasCommand(function_name) ||
           static_cast<bool>(
               m_jmpc_list.getCommand(function_name)->getCommandType() &
               JsonMessageCommand::NormalType))) {
        co_return makeErrorMessage("You haven't logged in!");
      }
    }

    if (function_name == "login") {
      co_return login(UserID(param["user_id"].getInt()),
                      param["password"].getString(),
                      param["device"].getString(), socket_service);
    }

    if (!m_jmpc_list.hasCommand(function_name)) {
      co_return makeErrorMessage(
          "There isn't a function that matches the name!");
    }

    // Find the command that matches the function name
    auto command_ptr = m_jmpc_list.getCommand(function_name);
    const qjson::dict_t &param_dict = param.getDict();
    // Check whether the type of json values match the options
    const auto &options = command_ptr->getOption();
    for (const auto &[name, type] : options) {
      auto local_iter = param_dict.find(std::string_view(name));
      if (local_iter == param_dict.cend()) {
        auto fmt = std::format("Lost a parameter: {}.", name);
        co_return makeErrorMessage(fmt);
      }
      if (local_iter->second.getType() != type) {
        auto fmt = std::format("Wrong parameter type: {}.", name);
        co_return makeErrorMessage(fmt);
      }
    }

    UserID user_id;
    {
      // Get the ID of this user
      std::shared_lock id_lock(m_user_id_mutex);
      user_id = m_user_id;
    }

    // This function is used to execute the command asynchronously
    auto async_invoke = [](auto executor,
                           std::shared_ptr<JsonMessageCommand> command_ptr,
                           const UserID &user_id, qjson::JObject param,
                           auto &&token) {
      return asio::async_initiate<decltype(token),
                                  void(std::error_code, qjson::JObject)>(
          [](auto handler, auto executor,
             std::shared_ptr<JsonMessageCommand> command_ptr, UserID user_id,
             qjson::JObject param) {
            asio::post(executor, [handler = std::move(handler),
                                  command_ptr = std::move(command_ptr),
                                  user_id = std::move(user_id),
                                  param = std::move(param)]() mutable {
              try {
                handler({}, command_ptr->execute(user_id, std::move(param)));
              } catch (const std::system_error &e) {
                handler(e.code(), qjson::JObject{});
              } catch (...) {
                handler(std::error_code(asio::error::fault), qjson::JObject{});
              }
            });
          },
          token, std::move(executor), std::move(command_ptr), user_id,
          std::move(param));
    };

    co_return co_await async_invoke(co_await asio::this_coro::executor,
                                    std::move(command_ptr), user_id,
                                    std::move(param), asio::use_awaitable);
  } catch (const std::exception &e) {
#ifndef _DEBUG
    co_return makeErrorMessage("Unknown error occured!");
#else
    co_return makeErrorMessage(std::string("Unknown error occured: ") +
                               e.what());
#endif
  }
}

qjson::JObject
JsonMessageProcessImpl::login(const UserID &user_id, std::string_view password,
                              std::string_view device,
                              const SocketService &socket_service) {
  if (!serverManager.hasUser(user_id)) {
    return makeErrorMessage("The user ID or password is wrong!");
  }

  auto user = serverManager.getUser(user_id);

  if (user->isUserPassword(password)) {
    // check device type
    if (std::string_view(device) == "PersonalComputer") {
      serverManager.modifyUserOfConnection(socket_service.get_connection_ptr(),
                                           user_id,
                                           DeviceType::PersonalComputer);
    } else if (std::string_view(device) == "Phone") {
      serverManager.modifyUserOfConnection(socket_service.get_connection_ptr(),
                                           user_id, DeviceType::Phone);
    } else if (std::string_view(device) == "Web") {
      serverManager.modifyUserOfConnection(socket_service.get_connection_ptr(),
                                           user_id, DeviceType::Web);
    } else {
      serverManager.modifyUserOfConnection(socket_service.get_connection_ptr(),
                                           user_id, DeviceType::Unknown);
    }

    auto returnJson = makeSuccessMessage("Successfully logged in!");
    std::unique_lock lock(m_user_id_mutex);
    this->m_user_id = user_id;

    serverLogger.debug("User ", user_id.getOriginValue(),
                       " logged into the server");

    return returnJson;
  }
  return makeErrorMessage("The user ID or password is wrong!");
}

qjson::JObject JsonMessageProcessImpl::login(std::string_view email,
                                             std::string_view password,
                                             std::string_view device) {
  if (!qls::RegexMatch::emailMatch(std::string_view(email))) {
    return makeErrorMessage("Email is invalid");
  }

  // Not completed
  return makeErrorMessage("This function is incomplete.");
}

// -----------------------------------------------------------------------------------------------
// json process
// -----------------------------------------------------------------------------------------------

UserID JsonMessageProcess::getLocalUserID() const {
  return m_process->getLocalUserID();
}

asio::awaitable<qjson::JObject>
JsonMessageProcess::processJsonMessage(const qjson::JObject &json,
                                       const SocketService &socket_service) {
  co_return co_await m_process->processJsonMessage(json, socket_service);
}

} // namespace qls
