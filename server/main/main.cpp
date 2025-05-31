#include <logger.hpp>

#include "Ini.h"
#include "init.h"
#include "manager.h"

// server log system
Log::Logger serverLogger;
// ini config
qini::INIObject serverIni;
// manager
qls::Manager serverManager;

int main() {
  using namespace std::chrono;

  int code = qls::init();
  return code;
}