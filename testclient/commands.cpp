#include "commands.h"

#include <iostream>
#include <userid.hpp>

namespace qls {

class HelpCommand : public Command {
public:
  HelpCommand(CommandManager &cm) : m_command_manager(cm) {}
  ~HelpCommand() = default;

  opt::Option getOption() const {
    opt::Option option;
    option.add("name", opt::Option::OptionType::OPT_OPTIONAL);
    return option;
  }

  void execute(Session &session, const opt::Option &option) {
    if (option.has_opt_with_value("name")) {
      std::string commandName = option.get_string("name");
      if (!m_command_manager.canFindCommand(commandName)) {
        std::cout << "Could not find command: " << commandName << '\n';
      }
      m_command_manager.getCommand(commandName)->getOption().show();
      return;
    }

    std::shared_lock lock(m_command_manager.m_command_map_mutex);
    std::cout << "help [--name=(function name)]\n";
    for (const auto &[commandName, commandPtr] :
         std::as_const(m_command_manager.m_command_map)) {
      std::cout << commandName << '\n';
    }
  }

private:
  CommandManager &m_command_manager;
};

class ExitCommand : public Command {
public:
  ExitCommand() = default;
  ~ExitCommand() = default;

  opt::Option getOption() const { return {}; }

  void execute(Session &session, const opt::Option &option) { exit(0); }
};

class RegisterUserCommand : public Command {
public:
  RegisterUserCommand() = default;
  ~RegisterUserCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("email", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    qls::UserID user_id;
    if (session.registerUser(opt.get_string("email"),
                             opt.get_string("password"), user_id))
      std::cout << "Successfully created a new user! User id is: " << user_id
                << '\n';
    else
      std::cout << "Failed to created a new user!\n";
  }
};

class LoginUserCommand : public Command {
public:
  LoginUserCommand() = default;
  ~LoginUserCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.loginUser(qls::UserID(opt.get_int("userid")),
                          opt.get_string("password")))
      std::cout << "Successfully logined a user!\n";
    else
      std::cout << "Failed to logined a user!\n";
  }
};

class CreateFriendApplicationCommand : public Command {
public:
  CreateFriendApplicationCommand() = default;
  ~CreateFriendApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.createFriendApplication(qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully sent a application to the user!\n";
    else
      std::cout << "Failed to send a application to the user!\n";
  }
};

class AcceptFriendApplicationCommand : public Command {
public:
  AcceptFriendApplicationCommand() = default;
  ~AcceptFriendApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.applyFriendApplication(qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully applied a application to the user!\n";
    else
      std::cout << "Failed to apply a application to the user!\n";
  }
};

class RejectFriendApplicationCommand : public Command {
public:
  RejectFriendApplicationCommand() = default;
  ~RejectFriendApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.rejectFriendApplication(qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully rejected a application to the user!\n";
    else
      std::cout << "Failed to reject a application to the user!\n";
  }
};

class CreateGroupCommand : public Command {
public:
  CreateGroupCommand() = default;
  ~CreateGroupCommand() = default;

  opt::Option getOption() const { return {}; }

  void execute(Session &session, const opt::Option &opt) {
    if (session.createGroup())
      std::cout << "Successfully created a group!\n";
    else
      std::cout << "Failed to create a group!\n";
  }
};

class CreateGroupApplicationCommand : public Command {
public:
  CreateGroupApplicationCommand() = default;
  ~CreateGroupApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("groupid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.createGroupApplication(qls::GroupID(opt.get_int("groupid"))))
      std::cout << "Successfully sent a application to the group!\n";
    else
      std::cout << "Failed to send a application to the group!\n";
  }
};

class AcceptGroupApplicationCommand : public Command {
public:
  AcceptGroupApplicationCommand() = default;
  ~AcceptGroupApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("groupid", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.applyGroupApplication(qls::GroupID(opt.get_int("groupid")),
                                      qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully applied a application to the group!\n";
    else
      std::cout << "Failed to apply a application to the group!\n";
  }
};

class RejectGroupApplicationCommand : public Command {
public:
  RejectGroupApplicationCommand() = default;
  ~RejectGroupApplicationCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("groupid", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.rejectGroupApplication(qls::GroupID(opt.get_int("groupid")),
                                       qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully applied a application to the group!\n";
    else
      std::cout << "Failed to apply a application to the group!\n";
  }
};

class SendFriendMessageCommand : public Command {
public:
  SendFriendMessageCommand() = default;
  ~SendFriendMessageCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("message", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.sendFriendMessage(qls::UserID(opt.get_int("userid")),
                                  opt.get_string("message")))
      std::cout << "Successfully sent a message to your friend!\n";
    else
      std::cout << "Failed to send a message to your friend!\n";
  }
};

class SendGroupMessageCommand : public Command {
public:
  SendGroupMessageCommand() = default;
  ~SendGroupMessageCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("groupid", opt::Option::OptionType::OPT_REQUIRED);
    opt.add("message", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.sendGroupMessage(qls::GroupID(opt.get_int("groupid")),
                                 opt.get_string("message")))
      std::cout << "Successfully sent a message to the group!\n";
    else
      std::cout << "Failed to send a message to the group!\n";
  }
};

class RemoveFriendCommand : public Command {
public:
  RemoveFriendCommand() = default;
  ~RemoveFriendCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.removeFriend(qls::UserID(opt.get_int("userid"))))
      std::cout << "Successfully removed a friend!\n";
    else
      std::cout << "Failed to remove a friend!\n";
  }
};

class LeaveGroupCommand : public Command {
public:
  LeaveGroupCommand() = default;
  ~LeaveGroupCommand() = default;

  opt::Option getOption() const {
    opt::Option opt;
    opt.add("groupid", opt::Option::OptionType::OPT_REQUIRED);
    return opt;
  }

  void execute(Session &session, const opt::Option &opt) {
    if (session.leaveGroup(qls::GroupID(opt.get_int("groupid"))))
      std::cout << "Successfully left a group!\n";
    else
      std::cout << "Failed to leave a group!\n";
  }
};

CommandManager::CommandManager() {
  addCommand("exit", std::make_shared<ExitCommand>());
  addCommand("help", std::make_shared<HelpCommand>(*this));
  addCommand("registerUser", std::make_shared<RegisterUserCommand>());
  addCommand("loginUser", std::make_shared<LoginUserCommand>());

  addCommand("createFriendApplication",
             std::make_shared<CreateFriendApplicationCommand>());
  addCommand("acceptFriendApplication",
             std::make_shared<AcceptFriendApplicationCommand>());
  addCommand("rejectFriendApplication",
             std::make_shared<RejectFriendApplicationCommand>());

  addCommand("createGroup", std::make_shared<CreateGroupCommand>());
  addCommand("createGroupApplication",
             std::make_shared<CreateGroupApplicationCommand>());
  addCommand("acceptGroupApplication",
             std::make_shared<AcceptGroupApplicationCommand>());
  addCommand("rejectGroupApplication",
             std::make_shared<RejectGroupApplicationCommand>());

  addCommand("sendFriendMessage", std::make_shared<SendFriendMessageCommand>());
  addCommand("sendGroupMessage", std::make_shared<SendGroupMessageCommand>());

  addCommand("removeFriend", std::make_shared<RemoveFriendCommand>());
  addCommand("leaveGroup", std::make_shared<LeaveGroupCommand>());
}

bool CommandManager::addCommand(std::string_view commandName,
                                const std::shared_ptr<Command> &commandPtr) {
  std::unique_lock lock(m_command_map_mutex);
  if (m_command_map.find(commandName) != m_command_map.cend()) {
    return false;
  }
  m_command_map.emplace(commandName, commandPtr);
  return true;
}

bool CommandManager::removeCommand(std::string_view commandName) {
  std::unique_lock lock(m_command_map_mutex);
  auto iter = m_command_map.find(commandName);
  if (iter == m_command_map.cend()) {
    return false;
  }
  m_command_map.erase(iter);
  return true;
}

bool CommandManager::canFindCommand(std::string_view commandName) const {
  std::shared_lock lock(m_command_map_mutex);
  return m_command_map.find(commandName) != m_command_map.cend();
}

std::shared_ptr<Command>
CommandManager::getCommand(std::string_view commandName) const {
  std::shared_lock lock(m_command_map_mutex);
  auto iter = m_command_map.find(commandName);
  if (iter == m_command_map.cend())
    throw std::logic_error(std::string(commandName) + " does not exist");
  return iter->second;
}

} // namespace qls
