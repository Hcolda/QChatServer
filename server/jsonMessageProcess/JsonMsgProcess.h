#ifndef JSON_MESSAGE_PROCESS_H
#define JSON_MESSAGE_PROCESS_H

#include <asio.hpp>
#include <Json.h>
#include <memory>

#include "userid.hpp"
#include "socketFunctions.h"

namespace qls
{

class JsonMessageProcessImpl;

class JsonMessageProcess final
{
public:
    JsonMessageProcess(UserID user_id);
    ~JsonMessageProcess();

    UserID getLocalUserID() const;
    asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject& json, const SocketService& sf);
    
private:
    std::unique_ptr<JsonMessageProcessImpl> m_process;
};

} // namespace qls

#endif // !JSON_MESSAGE_PROCESS_H