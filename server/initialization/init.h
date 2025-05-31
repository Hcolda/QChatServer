#ifndef INIT_H
#define INIT_H

#include <Ini.h>

namespace qls {

class Init final {
public:
  Init() = default;
  ~Init() = default;

  /// @brief Create config file
  static void createConfig();

  /// @brief Read config file
  /// @return Ini object
  static qini::INIObject readConfig();
};

int init();

} // namespace qls

#endif // !INIT_H
