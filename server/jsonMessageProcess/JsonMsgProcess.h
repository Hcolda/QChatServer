#ifndef JSON_MESSAGE_PROCESS_H
#define JSON_MESSAGE_PROCESS_H

#include <Json.h>
#include <asio.hpp>
#include <memory>

#include "socketFunctions.h"
#include "userid.hpp"

namespace qls {

class JsonMessageProcessImpl;

class JsonMessageProcess final {
public:
  JsonMessageProcess(UserID user_id);
  ~JsonMessageProcess();

  UserID getLocalUserID() const;
  asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject &json,
                                                     const SocketService &sf);

private:
  std::unique_ptr<JsonMessageProcessImpl> m_process;
};

} // namespace qls

#endif // !JSON_MESSAGE_PROCESS_H