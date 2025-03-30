#include "JsonMsgProcessCommand.h"

#include <unordered_set>
#include <logger.hpp>

#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"
#include "groupid.hpp"
#include "userid.hpp"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls
{

qjson::JObject RegisterCommand::execute(UserID executor, qjson::JObject parameters)
{
    std::string email = parameters["email"].getString();
    std::string password = parameters["password"].getString();

    if (!RegexMatch::emailMatch(email)) {
        return makeErrorMessage("Email is invalid");
    }

    auto ptr = serverManager.addNewUser();
    ptr->firstUpdateUserPassword(password);
    ptr->updateUserEmail(email);

    qjson::JObject returnJson = makeSuccessMessage("Successfully created a new user!");
    UserID id = ptr->getUserID();
    returnJson["user_id"] = id.getOriginValue();

    serverLogger.debug("Registered new user: ", id.getOriginValue());
    return returnJson;
}

qjson::JObject HasUserCommand::execute(UserID executor, qjson::JObject parameters)
{
    bool has_user = serverManager.hasUser(UserID(parameters["user_id"].getInt()));
    auto returnJson = makeSuccessMessage("Successfully get a result!");
    returnJson["has_user"] = has_user;
    return returnJson;
}

qjson::JObject SearchUserCommand::execute(UserID executor, qjson::JObject parameters)
{
    return makeErrorMessage("This function is incomplete.");
}

qjson::JObject AddFriendCommand::execute(UserID executor, qjson::JObject parameters)
{
    UserID friend_id = UserID(parameters["user_id"].getInt());

    if (!serverManager.hasUser(friend_id))
        return makeErrorMessage("UserID is invalid!");

    if (serverManager.getUser(executor)->addFriend(friend_id))
    {
        serverLogger.debug("User ", executor.getOriginValue(), " sent a friend request to user ", friend_id.getOriginValue());
        return makeSuccessMessage("Successfully sent application!");
    }
    else return makeErrorMessage("Can't send application");
}

qjson::JObject AcceptFriendVerificationCommand::execute(UserID executor, qjson::JObject parameters)
{
    UserID user_id = UserID(parameters["user_id"].getInt());

    if (!serverManager.hasUser(user_id))
        return makeErrorMessage("UserID is invalid!");

    if (serverManager.getUser(executor)->acceptFriend(user_id))
        serverLogger.debug("User ", executor.getOriginValue(), " apply user \"", user_id.getOriginValue(), "\"'s friend request");
    return makeSuccessMessage("Successfully added a friend!");
}

qjson::JObject RejectFriendVerificationCommand::execute(UserID executor, qjson::JObject parameters)
{
    UserID user_id = UserID(parameters["user_id"].getInt());

    if (!serverManager.hasUser(user_id))
        return makeErrorMessage("UserID is invalid!");

    if (!serverManager.getUser(executor)->rejectFriend(user_id)) {
        return makeErrorMessage("Failed to reject!"); 
    }

    serverLogger.debug("User ", executor.getOriginValue(), " reject user \"", user_id.getOriginValue(), "\"'s friend request");
    return makeSuccessMessage("Successfully rejected a friend verification!");
}

qjson::JObject GetFriendListCommand::execute(UserID executor, qjson::JObject parameters)
{
    auto set = serverManager.getUser(executor)->getFriendList();
    qjson::JObject returnJson = makeSuccessMessage("Successfully obtained friend list!");

    for (const auto& i : set) {
        returnJson["friend_list"].push_back(i.getOriginValue());
    }
    serverLogger.debug("User ", executor.getOriginValue(), " get a friend list");

    return returnJson;
}

qjson::JObject GetFriendVerificationListCommand::execute(UserID executor, qjson::JObject parameters)
{
    auto map = serverManager.getUser(executor)->getFriendVerificationList();
    qjson::JObject localVector;
    for (const auto& [user_id, user_struct] : map) {
        qjson::JObject localJson;
        localJson["user_id"] = user_id.getOriginValue();
        localJson["verification_type"] = (int)user_struct.verification_type;
        localJson["message"] = user_struct.message;

        localVector.push_back(std::move(localJson));
    }

    auto returnJson = makeSuccessMessage("Successfully obtained verification list!");
    returnJson["result"] = localVector;

    serverLogger.debug("User ", executor.getOriginValue(), " get a friend verification list");

    return returnJson;
}

qjson::JObject RemoveFriendCommand::execute(UserID executor, qjson::JObject parameters)
{
    UserID user_id = UserID(parameters["user_id"].getInt());

    if (!serverManager.hasUser(user_id))
        return makeErrorMessage("UserID is invalid!");

    if (!serverManager.getUser(executor)->removeFriend(user_id))
        return makeErrorMessage("Failed to remove a friend!");

    serverLogger.debug("User ", executor.getOriginValue(), " remove a friend: ", user_id.getOriginValue());
    return makeSuccessMessage("Successfully removed a friend");
}

qjson::JObject AddGroupCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());

    if (!serverManager.hasGroupRoom(group_id))
        return makeErrorMessage("GroupID is invalid!");
        
    if (serverManager.getUser(executor)->addGroup(group_id)) {
        serverLogger.debug("User ", executor.getOriginValue(), " sent a group request to group ", group_id.getOriginValue());
        return makeSuccessMessage("Successfully sent a group application!");
    }
    else return makeErrorMessage("Failed to send a group application!");
}

qjson::JObject AcceptGroupVerificationCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());
    UserID user_id = UserID(parameters["user_id"].getInt());

    if (serverManager.getUser(executor)->acceptGroup(group_id, user_id)) {
        serverLogger.debug("User ", executor.getOriginValue(), " accept user \"", user_id.getOriginValue(), "\"'s group request");
        return makeSuccessMessage("Successfully accepted a group application!");
    }
    else return makeErrorMessage("Failed to accept a group application!"); 
}

qjson::JObject RejectGroupVerificationCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());
    UserID user_id = UserID(parameters["user_id"].getInt());

    if (serverManager.getUser(executor)->rejectGroup(group_id, user_id)) {
        serverLogger.debug("User ", executor.getOriginValue(), " reject user \"", user_id.getOriginValue(), "\"'s group request");
        return makeSuccessMessage("Successfully reject a group verfication!");
    }
    else return makeErrorMessage("Failed to reject a group application!"); 
}

qjson::JObject GetGroupListCommand::execute(UserID executor, qjson::JObject parameters)
{
    auto set = std::move(serverManager.getUser(executor)->getGroupList());
    qjson::JObject returnJson = makeSuccessMessage("Successfully obtained group list!");

    for (const auto& i : set) {
        returnJson["friend_list"].push_back(i.getOriginValue());
    }

    serverLogger.debug("User ", executor.getOriginValue(), " get a group list");

    return returnJson;
}

qjson::JObject GetGroupVerificationListCommand::execute(UserID executor, qjson::JObject parameters)
{
    auto map = std::move(serverManager.getUser(executor)->getGroupVerificationList());
    auto returnJson = makeSuccessMessage("Successfully obtained verification list!");
    for (const auto& [group_id, user_struct] : map) {
        auto group = std::to_string(group_id.getOriginValue());
        returnJson["result"][group.c_str()]["user_id"] = user_struct.user_id.getOriginValue();
        returnJson["result"][group.c_str()]["verification_type"] = (int)user_struct.verification_type;
        returnJson["result"][group.c_str()]["message"] = user_struct.message;
    }

    serverLogger.debug("User ", executor.getOriginValue(), " get a group verification list");

    return returnJson;
}

qjson::JObject SendFriendMessageCommand::execute(UserID executor, qjson::JObject parameters)
{
    UserID friend_id = UserID(parameters["friend_id"].getInt());
    std::string msg = parameters["message"].getString();

    if (!serverManager.hasUser(friend_id))
        return makeErrorMessage("UserID is invalid!");

    if (!serverManager.getUser(executor)->userHasFriend(friend_id))
        return makeErrorMessage("You don't have this friend!");

    // sending a message
    serverManager.getPrivateRoom(
            serverManager.getPrivateRoomId(
                executor, friend_id))->sendMessage(msg, executor);

    serverLogger.debug("User ", executor.getOriginValue(), " sent a message to user ", friend_id.getOriginValue());

    return makeSuccessMessage("Successfully sent a message!");
}

qjson::JObject SendGroupMessageCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());
    std::string msg = parameters["message"].getString();

    if (!serverManager.hasGroupRoom(group_id))
        return makeErrorMessage("GroupID is invalid!");
        
    if (!serverManager.getUser(executor)->userHasGroup(group_id))
        return makeErrorMessage("You don't have this group!");

    serverManager.getGroupRoom(group_id)->sendMessage(executor, msg);
    serverLogger.debug("User ", executor.getOriginValue(), " sent a message to group ", group_id.getOriginValue());

    return makeSuccessMessage("Successfully sent a message!");
}

qjson::JObject CreateGroupCommand::execute(UserID executor, qjson::JObject parameters)
{
    try {
        GroupID group_id = serverManager.getUser(executor)->createGroup();
        qjson::JObject json = makeSuccessMessage("Successfully create a group!");
        json["group_id"] = group_id.getOriginValue();
        return json;
    } catch(...) {
        return makeErrorMessage("Failed to create a group!");
    }
}

qjson::JObject RemoveGroupCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());
    if (!serverManager.getUser(executor)->removeGroup(group_id))
        return makeErrorMessage("Failed to remove a group!");
    return makeSuccessMessage("Successfully removed a group!");
}

qjson::JObject LeaveGroupCommand::execute(UserID executor, qjson::JObject parameters)
{
    GroupID group_id = GroupID(parameters["group_id"].getInt());
    return makeErrorMessage("This function is incomplete.");
}

} // namespace qls
