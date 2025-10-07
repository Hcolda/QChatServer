#include "input.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "inputCommands.h"
#include "logger.hpp"

// 服务器log系统
extern Log::Logger serverLogger;

using namespace qls;

template <std::size_t N, std::size_t N2>
constexpr static void getTargetName(const char (&data)[N], char (&out)[N2]) {
  std::size_t size = std::strlen(data);
  for (std::size_t i = 0; i < size; ++i) {
    if (data[i] == '_') {
      out[i] = ' ';
    } else {
      out[i] = data[i];
    }
  }
}

#define MERGE(x, y) x##y

#define SET_A_COMMAND(name)                                                    \
  {                                                                            \
    char local[sizeof(#name)]{0};                                              \
    getTargetName(#name, local);                                               \
    m_command_map.emplace(local, std::make_unique<MERGE(name, _command)>());   \
  }

class InputImpl {
public:
  InputImpl() {
    SET_A_COMMAND(stop);
    SET_A_COMMAND(show_user);
  }

  ~InputImpl() = default;

  bool input(std::string_view param) {
    std::string first_word;
    std::string_view command = param;
    std::string_view::const_iterator iter = command.cbegin();
    while (iter != command.cend() && *iter == ' ') {
      ++iter;
    }
    while (iter != command.cend() && *iter != '-') {
      first_word += *(iter++);
    }

    first_word = strip(first_word);
    if (first_word.empty()) {
      return true;
    }
    std::string arguments(iter, command.cend());

    if (auto words = split(first_word); words[0] == "help") {
      if (first_word.size() <= sizeof("help")) {
        for (auto i = m_command_map.begin(); i != m_command_map.end(); ++i) {
          serverLogger.info(i->first, ": ",
                            i->second->registerCommand().description);
        }
        return true;
      }
      std::string origin_command = strip(first_word.substr(sizeof("help")));
      auto iter = m_command_map.find(origin_command);
      if (iter != m_command_map.end()) {
        serverLogger.info(origin_command, ": ",
                          iter->second->registerCommand().description);
        return true;
      }
      bool has_find = false;
      for (auto i = m_command_map.begin(); i != m_command_map.end(); ++i) {
        if (i->first.substr(0, origin_command.size()) == origin_command) {
          has_find = true;
          serverLogger.info(i->first, ": ",
                            i->second->registerCommand().description);
        }
      }
      if (!has_find) {
        serverLogger.warning("Command not existed: ", origin_command);
      }
      return true;
    }

    auto map_iter = m_command_map.find(first_word);
    if (map_iter == m_command_map.cend()) {
      serverLogger.warning("Command not existed: ", first_word);
      return true;
    }

    auto opt = map_iter->second->registerCommand().option;
    opt.add("help", opt::Option::OptionType::OPT_OPTIONAL);
    opt.add("h", opt::Option::OptionType::OPT_OPTIONAL);
    opt.parse(split(arguments));
    if (opt.has_opt_with_value("help") || opt.has_opt_with_value("h")) {
      serverLogger.info(map_iter->first, ": ",
                        map_iter->second->registerCommand().description);
      return true;
    }
    map_iter->second->setArguments(opt);
    return map_iter->second->execute();

    serverLogger.warning("Command not existed: ", first_word);
    return true;
  }

private:
  static std::string strip(std::string_view param) {
    std::string_view data = param;
    std::string_view::const_iterator first = data.cbegin();
    auto last = data.crbegin();

    while (first != data.cend() && *first == ' ') {
      ++first;
    }
    while (last != data.crend() && *last == ' ') {
      ++last;
    }

    if (first >= last.base()) {
      return {};
    }
    return {first, last.base()};
  }

  static std::vector<std::string> split(std::string_view param) {
    std::string_view data = param;
    std::vector<std::string> dataList;

    long long begin = -1;
    long long iter = 0;

    for (; static_cast<std::size_t>(iter) < data.size(); iter++) {
      if (data[iter] == ' ') {
        if ((iter - begin - 1) > 0) {
          dataList.emplace_back(data.begin() + (begin + 1),
                                data.begin() + iter);
        }
        begin = iter;
      }
    }
    dataList.emplace_back(data.begin() + (begin + 1), data.begin() + iter);

    return dataList;
  }

  std::unordered_map<std::string, std::unique_ptr<qls::Command>> m_command_map;
};

Input::Input() : m_impl(std::make_unique<InputImpl>()) {}

Input::~Input() = default;

void Input::init() {}

bool Input::input(std::string_view command) { return m_impl->input(command); }
