#include "session.h"

#include <vector>
#include <iostream>
#include <Json.h>

namespace qls
{

struct SessionImpl
{
    Network& network;
    UserID user_id;
    bool has_login = false;
};

static inline qjson::JObject makeJsonFunctionDataPackage(std::string_view functionName,
    std::vector<std::pair<std::string, qjson::JObject>> list)
{
    qjson::JObject json(qjson::JValueType::JDict);
    json["function"] = functionName;
    json["parameters"] = qjson::JObject(qjson::JValueType::JDict);
    for (auto [parameterName, parameterValue]: list)
    {
        json["parameters"][parameterName.c_str()] = parameterValue;
    }
    return json;
}

static inline qjson::JObject makeJsonFunctionDataPackage(std::string_view functionName)
{
    qjson::JObject json(qjson::JValueType::JDict);
    json["function"] = std::string(functionName);
    json["parameters"] = qjson::JObject(qjson::JValueType::JDict);
    return json;
}

static inline qjson::JObject readJsonFunctionDataPackage(const std::shared_ptr<qls::DataPackage>& package)
{
    return qjson::JParser::fastParse(package->getData());
}

Session::Session(Network& network):
    m_impl(std::make_unique<SessionImpl>(network)) {}

Session::~Session() noexcept = default;

bool Session::registerUser(std::string_view email, std::string_view password, UserID& newUserID)
{
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("register", {{"email", email}, {"password", password}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    bool returnState = returnJson["state"].getString() == "success";
    if (returnState)
        newUserID = UserID(returnJson["user_id"].getInt());
    return returnState;
}

bool Session::loginUser(UserID user_id, std::string_view password)
{
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("login",
            {{"user_id", user_id.getOriginValue()}, {"password", password}, {"device", "PersonalComputer"}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    bool returnState = returnJson["state"].getString() == "success";
    if (returnState)
    {
        m_impl->user_id = user_id;
        m_impl->has_login = true;
    }
    return returnState;
}

bool Session::createFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("add_friend", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::applyFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("accept_friend_verification", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::rejectFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("reject_friend_verification", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::createGroup()
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("create_group")),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << "-- group id: " << returnJson["group_id"].getInt() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::createGroupApplication(GroupID group_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("add_group", {{"group_id", group_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::applyGroupApplication(GroupID group_id, UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("accept_group_verification",
            {{"group_id", group_id.getOriginValue()},
             {"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::rejectGroupApplication(GroupID group_id, UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("reject_group_verification",
            {{"group_id", group_id.getOriginValue()},
             {"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::sendFriendMessage(UserID user_id, std::string_view message)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("send_friend_message",
        {{"user_id", user_id.getOriginValue()},
         {"message", message}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::sendGroupMessage(GroupID group_id, std::string_view message)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("send_group_message",
        {{"group_id", group_id.getOriginValue()},
         {"message", message}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::removeFriend(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("remove_friend", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::leaveGroup(GroupID group_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("leave_group", {{"group_id", group_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

} // namespace qls
