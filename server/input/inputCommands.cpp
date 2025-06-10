#include "inputCommands.h"

#include <format>

#include <option.hpp>

#include "Ini.h"
#include "logger.hpp"
#include "manager.h"
#include "network.h"

// 服务器log系统
extern Log::Logger serverLogger;
// ini配置
extern qini::INIObject serverIni;
// manager
extern qls::Manager serverManager;

namespace qls {

bool stop_command::execute() {
  serverManager.getServerNetwork().stop();
  return false;
}

CommandInfo stop_command::registerCommand() { return {{}, "stop server"}; }

bool show_user_command::execute() {
  serverManager.getUserList([](const auto &map) {
    serverLogger.info("User data list: \n");
    for (const auto &[user_id, user] : map) {
      serverLogger.info(std::format("user id: {}, name: {}\n",
                                    user_id.getOriginValue(),
                                    user->getUserName()));
    }
  });
  return true;
}

CommandInfo show_user_command::registerCommand() {
  return {{}, "show user's infomation"};
}

} // namespace qls
