#include "session.h"

#include <Json.h>
#include <iostream>
#include <vector>

namespace qls {

struct SessionImpl {
  Network &network;
  UserID user_id;
  bool has_login = false;
};

static inline qjson::JObject makeJsonFunctionDataPackage(
    string_param functionName,
    std::vector<std::pair<std::string, qjson::JObject>> list) {
  qjson::JObject json(qjson::JValueType::JDict);
  json["function"] = std::string_view(functionName);
  json["parameters"] = qjson::JObject(qjson::JValueType::JDict);
  for (auto [parameterName, parameterValue] : list) {
    json["parameters"][parameterName] = parameterValue;
  }
  return json;
}

static inline qjson::JObject
makeJsonFunctionDataPackage(string_param functionName) {
  qjson::JObject json(qjson::JValueType::JDict);
  json["function"] = std::string_view(functionName);
  json["parameters"] = qjson::JObject(qjson::JValueType::JDict);
  return json;
}

static inline qjson::JObject
readJsonFunctionDataPackage(const DataPackagePtr &package) {
  return qjson::to_json(package->getData());
}

Session::Session(Network &network)
    : m_impl(std::make_unique<SessionImpl>(network)) {}

Session::~Session() noexcept = default;

bool Session::registerUser(string_param email, string_param password,
                           UserID &newUserID) {
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "register", {{"email", std::string_view(email)},
                               {"password", std::string_view(password)}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  bool returnState = returnJson["state"].getString() == "success";
  if (returnState)
    newUserID = UserID(returnJson["user_id"].getInt());
  return returnState;
}

bool Session::loginUser(UserID user_id, string_param password) {
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "login", {{"user_id", user_id.getOriginValue()},
                            {"password", std::string_view(password)},
                            {"device", "PersonalComputer"}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  bool returnState = returnJson["state"].getString() == "success";
  if (returnState) {
    m_impl->user_id = user_id;
    m_impl->has_login = true;
  }
  return returnState;
}

bool Session::createFriendApplication(UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "add_friend", {{"user_id", user_id.getOriginValue()}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::applyFriendApplication(UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "accept_friend_verification",
                                   {{"user_id", user_id.getOriginValue()}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::rejectFriendApplication(UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "reject_friend_verification",
                                   {{"user_id", user_id.getOriginValue()}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::createGroup() {
  if (!m_impl->has_login)
    return false;
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage("create_group").to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString()
            << "-- group id: " << returnJson["group_id"].getInt() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::createGroupApplication(GroupID group_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "add_group", {{"group_id", group_id.getOriginValue()}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::applyGroupApplication(GroupID group_id, UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "accept_group_verification",
                                   {{"group_id", group_id.getOriginValue()},
                                    {"user_id", user_id.getOriginValue()}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::rejectGroupApplication(GroupID group_id, UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "reject_group_verification",
                                   {{"group_id", group_id.getOriginValue()},
                                    {"user_id", user_id.getOriginValue()}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::sendFriendMessage(UserID user_id, string_param message) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "send_friend_message",
                                   {{"user_id", user_id.getOriginValue()},
                                    {"message", std::string_view(message)}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::sendGroupMessage(GroupID group_id, string_param message) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage = m_impl->network
                           .send_data_with_result_n_option(
                               makeJsonFunctionDataPackage(
                                   "send_group_message",
                                   {{"group_id", group_id.getOriginValue()},
                                    {"message", std::string_view(message)}})
                                   .to_string(),
                               [](DataPackagePtr &package) {
                                 package->type = DataPackage::Text;
                               })
                           .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::removeFriend(UserID user_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "remove_friend", {{"user_id", user_id.getOriginValue()}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

bool Session::leaveGroup(GroupID group_id) {
  if (!m_impl->has_login)
    return false;
  auto returnPackage =
      m_impl->network
          .send_data_with_result_n_option(
              makeJsonFunctionDataPackage(
                  "leave_group", {{"group_id", group_id.getOriginValue()}})
                  .to_string(),
              [](DataPackagePtr &package) {
                package->type = DataPackage::Text;
              })
          .get();
  auto returnJson = readJsonFunctionDataPackage(returnPackage);
  std::cout << returnJson["message"].getString() << '\n';
  return returnJson["state"].getString() == "success";
}

} // namespace qls
