#include <atomic>
#include <exception>
#include <iostream>
#include <system_error>

#include "commands.h"
#include "network.h"
#include "session.h"
#include <groupid.hpp>
#include <option.hpp>
#include <userid.hpp>

static std::string strip(std::string_view data) {
  std::string_view::const_iterator first = data.cbegin();
  auto last = data.crbegin();

  while (first != data.cend() && *first == ' ')
    ++first;
  while (last != data.crend() && *last == ' ')
    ++last;

  if (first >= last.base())
    return {};
  return {first, last.base()};
}

static std::vector<std::string> split(std::string_view data) {
  std::vector<std::string> dataList;

  long long begin = -1;
  long long i = 0;

  for (; static_cast<std::size_t>(i) < data.size(); i++) {
    if (data[i] == ' ') {
      if ((i - begin - 1) > 0)
        dataList.emplace_back(data.begin() + (begin + 1), data.begin() + i);
      begin = i;
    }
  }
  dataList.emplace_back(data.begin() + (begin + 1), data.begin() + i);

  return dataList;
}

qls::CommandManager command_manager;

int main() {
  qls::Network network;
  qls::Session session(network);
  std::atomic<bool> can_be_used = false;

  network.add_connected_error_callback(
      "connected_error_callback", [](std::error_code ec) {
        std::cerr << "Connected error: " << ec.message() << '\n';
      });
  network.add_connected_callback("connected_callback", [&]() {
    std::cout << "Connected to server successfully!\n";
    can_be_used = true;
  });
  network.add_received_stdstring_callback(
      "received_stdstring_callback", [](std::string message) {
        auto pack = qls::DataPackage::stringToPackage(message);

        std::cout << "Message received:\n"
                  << "\tType: " << pack->type << '\n'
                  << "\tBody: " << pack->getData() << '\n';
      });

  try {
    network.connect();
  } catch (const std::exception &e) {
    std::cout << e.what() << '\n';
    return -1;
  }

  std::cout << "Connecting to server...\n";
  while (true) {
    if (!can_be_used) {
      using namespace std::chrono;
      std::this_thread::sleep_for(0.1s);
      continue;
    }
    char buffer[8192]{0};
    std::string command;
    std::cin.getline(buffer, sizeof(buffer) - 1);
    command = buffer;
    if (command == "")
      continue;

    std::vector<std::string> vec = split(strip(command));
    if (vec[0] == "stop") {
      network.stop();
      return 0;
    }
    try {
      if (!command_manager.canFindCommand(vec[0])) {
        std::cout << "Could not find a command: " << vec[0] << '\n';
        continue;
      }
      auto command = command_manager.getCommand(vec[0]);
      opt::Option opt = command->getOption();
      opt.parse(std::vector<std::string>{++vec.begin(), vec.end()});
      command->execute(session, opt);
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }
}
