# Json Message Map
这个readme专门记载服务器的json命令

1. **RegisterCommand**
这个命令是用于注册用户的
    - 传入格式
        ```json
        {
            "function": "register",
            "parameters": {
                "email": "Your email",
                "password": "Your password"
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully created a new user!",
                "user_id": 10000 // Your new user id
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

2. **HasUserCommand**
这个命令是用于查找用户id的
    - 传入格式
        ```json
        {
            "function": "has_user",
            "parameters": {
                "user_id": 10000 // The user id which you want to look for
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully get a result!",
                "has_user": true // or false: Have the user
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

3. **SearchUserCommand**
这个命令是用于查找用户名的 **（未完成）**
    - 传入格式
        ```json
        {
            "function": "search_user",
            "parameters": {
                "user_name": "The user name which you want to look for"
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully get a result!",
                "has_user": true // or false
                "user_id": 10000 // The user id for the user you wanna looking for
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

4. **AddFriendCommand**
这个命令是用于加其他用户好友的
    - 传入格式
        ```json
        {
            "function": "add_friend",
            "parameters": {
                "user_id": 10000 // The user id which you want to make friend with
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully sent application!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

5. **AcceptFriendVerificationCommand**
这个命令是用于同意其他用户好友的申请
    - 传入格式
        ```json
        {
            "function": "accept_friend_verification",
            "parameters": {
                "user_id": 10000 // The user id which you want to apply to make friend with
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully added a friend!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

6. **RejectFriendVerificationCommand**
这个命令是用于拒绝其他用户好友的申请
    - 传入格式
        ```json
        {
            "function": "reject_friend_verification",
            "parameters": {
                "user_id": 10000 // The user id which you want to reject to make friend with
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully rejected a friend verification!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

7. **GetFriendListCommand**
这个命令是用于获取此用户的好友列表
    - 传入格式
        ```json
        {
            "function": "get_friend_list",
            "parameters": {}
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully obtained friend list!",
                "friend_list": [10000, 10001] // and so on, about you friends' IDs
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

8. **GetFriendVerificationListCommand**
这个命令是用于获取此用户的好友申请列表
    - 传入格式
        ```json
        {
            "function": "get_friend_verification_list",
            "parameters": {}
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully obtained verification list!",
                "result": [
                    {
                        "user_id": 10000,
                        "verification_type": 1, // 1 for SEND, 2 for RECEIVED
                        "message": "the message for application" // not completed yet
                    }
                ]
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

9. **RemoveFriendCommand**
这个命令是用于删除好友的
    - 传入格式
        ```json
        {
            "function": "get_friend_verification_list",
            "parameters": {
                "user_id": 10000 // The user id of the user you wanna remove
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully removed a friend"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

10. **AddGroupCommand**
这个命令是用于添加群聊的
    - 传入格式
        ```json
        {
            "function": "add_group",
            "parameters": {
                "group_id": 10000 // The group id of the group you want to add
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully sent a group application!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

11. **AcceptGroupVerificationCommand**
这个命令是用于同意加入群聊用户的
    - 传入格式
        ```json
        {
            "function": "accept_group_verification",
            "parameters": {
                "group_id": 10000 // The group id of the group you want to apply the user
                "user_id": 10000 // The user id of the user you want to apply
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully accepted a group application!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

12. **RejectGroupVerificationCommand**
这个命令是用于拒绝加入群聊用户的
    - 传入格式
        ```json
        {
            "function": "reject_group_verification",
            "parameters": {
                "group_id": 10000 // The group id of the group you want to reject the user
                "user_id": 10000 // The user id of the user you want to reject
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully reject a group verfication!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

12. **GetGroupListCommand**
这个命令是用于获取此用户的群组列表的
    - 传入格式
        ```json
        {
            "function": "reject_group_verification",
            "parameters": {}
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully obtained group list!",
                "group_list": [
                    10000, 20000 // group IDs for the groups your joined
                ]
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

13. **GetGroupVerificationListCommand**
这个命令是用于获取此用户的群组用户申请列表的
    - 传入格式
        ```json
        {
            "function": "get_group_verification_list",
            "parameters": {}
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully obtained verification list!",
                "result": {
                    "10000": { // group id
                        "user_id": 10000, // user id
                        "verification_type": 1, // 1 for SEND, 2 for RECEIVE
                        "message": "Application message"
                    }
                }
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

14. **SendFriendMessageCommand**
这个命令是用于向其他用户发送消息的
    - 传入格式
        ```json
        {
            "function": "send_friend_message",
            "parameters": {
                "user_id": 10000, // user id
                "message": "The message you want to send"
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully sent a message!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

15. **SendGroupMessageCommand**
这个命令是用于向群组发送消息的
    - 传入格式
        ```json
        {
            "function": "send_group_message",
            "parameters": {
                "group_id": 10000, // group id
                "message": "The message you want to send"
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully sent a message!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

16. **CreateGroupCommand**
这个命令是用于创建新群聊的
    - 传入格式
        ```json
        {
            "function": "create_group",
            "parameters": {}
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully create a group!",
                "group_id": 10000 // The group id of the new group
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

17. **RemoveGroupCommand**
这个命令是用于删除群组的 **（群组主人使用）**
    - 传入格式
        ```json
        {
            "function": "remove_group",
            "parameters": {
                "group_id": 10000 // The group id of the group which will be removed
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully removed a group!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```

17. **LeaveGroupCommand**
这个命令是用于离开群组的 **（非群组主人使用）**
    - 传入格式
        ```json
        {
            "function": "remove_group",
            "parameters": {
                "group_id": 10000 // The group id of the group which will be left
            }
        }
        ```
    - 返回格式
        1. 成功
            ```json
            {
                "state": "success",
                "message": "Successfully left a group!"
            }
            ```
        2. 失败
            ```json
            {
                "state": "error",
                "message": "error message"
            }
            ```
