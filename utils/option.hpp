#ifndef OPTION_HPP
#define OPTION_HPP

// namespace option
#define NAMESPACE_OPTION_START namespace opt {
#define NAMESPACE_OPTION_END }

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

NAMESPACE_OPTION_START

class Option {
public:
  enum class OptionType { OPT_UNKNOWN = 0, OPT_NO, OPT_REQUIRED, OPT_OPTIONAL };

  Option() = default;
  ~Option() noexcept = default;

  Option(const Option &optClass)
      : m_opt_map(optClass.m_opt_map), m_args_map(optClass.m_args_map) {}

  Option(Option &&optClass) noexcept
      : m_opt_map(std::move(optClass.m_opt_map)),
        m_args_map(std::move(optClass.m_args_map)) {}

  Option &operator=(const Option &optClass) {
    if (this == &optClass)
      return *this;
    m_opt_map = optClass.m_opt_map;
    m_args_map = optClass.m_args_map;
    return *this;
  }

  Option &operator=(Option &&optClass) noexcept {
    if (this == &optClass)
      return *this;
    m_opt_map = std::move(optClass.m_opt_map);
    m_args_map = std::move(optClass.m_args_map);
    return *this;
  }

  /*
   * @brief Add an option
   * @param opt_name Name of the option
   * @param type Type of the option
   */
  void add(std::string_view opt_name, OptionType type) {
    if (type == OptionType::OPT_UNKNOWN)
      throw std::logic_error("Option type cannot be \"OPT_UNKNOWN\"");

    m_opt_map[std::string(opt_name)] = type;
  }

  /*
   * @brief Remove an option
   * @param opt_name Name of the option
   */
  void remove(std::string_view opt_name) {
    auto itor = m_opt_map.find(opt_name);
    if (itor != m_opt_map.cend())
      m_opt_map.erase(itor);
  }

  /*
   * @brief Parse command-line arguments
   * @param argc Number of arguments
   * @param argv Array of C-string arguments
   */
  void parse(int argc, char *const argv[]) {
    std::vector<std::string> args;

    for (int i = 0; i < argc; i++) {
      args.emplace_back(argv[i]);
    }

    parse(args);
  }

  /*
   * @brief Parse a command string
   * @param command Full command string to parse
   */
  void parse(std::string_view command) {
    std::string_view data = command;
    std::vector<std::string> dataList;

    long long begin = -1;
    long long i = 0;

    for (; std::size_t(i) < data.size(); i++) {
      if (data[i] == ' ') {
        if ((i - begin - 1) > 0)
          dataList.push_back(
              std::string(data.substr(begin + 1, i - begin - 1)));
        begin = i;
      }
    }
    dataList.push_back(std::string(data.substr(begin + 1, i - begin - 1)));

    parse(dataList);
  }

  /*
   * @brief Parse a list of string arguments
   * @param args List of string arguments
   */
  void parse(const std::vector<std::string> &args) {
    for (auto i = args.begin(); i != args.cend(); i++) {
      std::string_view arg = *i;

      if (arg.substr(0, 1) != "-") {
        // Ignore regular arguments
        continue;
      }
      if (arg.substr(0, 2) == "--") {
        // Long option parsing

        std::string str(arg.substr(2));
        if (str.empty())
          continue;

        auto pos = str.find('=');
        if (pos != std::string::npos) {
          // Found '='
          std::string opt_name = str.substr(0, pos);
          std::string value = str.substr(pos + 1);

          switch (get_type(opt_name)) {
          case OptionType::OPT_UNKNOWN:
            throw std::logic_error("No such option: " + std::string(opt_name));

          case OptionType::OPT_NO:
            throw std::logic_error("Option does not take an argument: " +
                                   std::string(opt_name));

          case OptionType::OPT_OPTIONAL:
          case OptionType::OPT_REQUIRED:
            m_args_map[opt_name] = value;
            break;

          default:
            break;
          }
        } else {
          std::string &opt_name = str;
          switch (get_type(opt_name)) {
          case OptionType::OPT_NO:
            m_args_map[opt_name] = "";
            break;

          case OptionType::OPT_OPTIONAL:
            if ((i + 1) != args.cend() && (i + 1)->substr(0, 1) != "-") {
              m_args_map[opt_name] = *(i + 1);
              i++;
            } else
              m_args_map[opt_name] = "";
            break;

          case OptionType::OPT_REQUIRED:
            if ((i + 1) != args.cend() && (i + 1)->substr(0, 1) != "-") {
              m_args_map[opt_name] = *(i + 1);
              i++;
              break;
            } else
              throw std::logic_error("Option requires an argument: " +
                                     std::string(opt_name));

          default:
            break;
          }
        }
      } else {
        // Short option parsing

        std::string str(arg.substr(1));
        if (str.empty())
          continue;
        if (str.find('=') != std::string::npos)
          throw std::logic_error("Invalid argument for option: " + str);

        std::string opt_name = str.substr(0, 1);

        switch (get_type(opt_name)) {
        case OptionType::OPT_NO:
          for (int i = 0; i < str.size(); i++) {
            std::string o(1, str[i]);
            if (get_type(o) != OptionType::OPT_NO)
              continue;
            m_args_map[o] = "";
          }
          break;

        case OptionType::OPT_OPTIONAL:
          if (str.size() > 1)
            m_args_map[opt_name] = str.substr(1);
          else if ((i + 1) != args.cend() && (i + 1)->substr(0, 1) != "-") {
            m_args_map[opt_name] = *(i + 1);
            i++;
          } else
            m_args_map[opt_name] = "";

          break;

        case OptionType::OPT_REQUIRED:
          if (str.size() > 1)
            m_args_map[opt_name] = str.substr(1);
          else if ((i + 1) != args.cend() && (i + 1)->substr(0, 1) != "-") {
            m_args_map[opt_name] = *(i + 1);
            i++;
            break;
          } else
            throw std::logic_error("Option requires an argument: " +
                                   std::string(opt_name));
          break;

        default:
          break;
        }
      }
    }
    for (const auto &[opt_name, opt_type] : m_opt_map) {
      if (opt_type == OptionType::OPT_REQUIRED &&
          m_args_map.find(opt_name) == m_args_map.cend())
        throw std::logic_error("Option requires an argument: " +
                               std::string(opt_name));
    }
  }

  /*
   * @brief Check if an option exists
   * @param opt Option name
   * @return True if the option exists, false otherwise
   */
  [[nodiscard]] bool has_opt(std::string_view opt) const {
    return m_opt_map.find(opt) != m_opt_map.cend();
  }

  /*
   * @brief Check if an option exists and has value
   * @param opt Option name
   * @return True if the option exists and has value, false otherwise
   */
  [[nodiscard]] bool has_opt_with_value(std::string_view opt) const {
    return m_opt_map.find(opt) != m_opt_map.cend() &&
           m_args_map.find(opt) != m_args_map.cend();
  }

  /*
   * @brief Get an option list
   * @return A list to show all the options
   */
  [[nodiscard]] auto get_opt_list() const { return m_opt_map; }

  /*
   * @brief Get the boolean value of an option
   * @param opt Option name
   * @return Boolean value of the option
   * @throw std::logic_error If the option does not exist or is not a boolean
   */
  [[nodiscard]] bool get_bool(std::string_view opt) const {
    if (!has_opt(opt))
      throw std::logic_error("No such option: " + std::string(opt));

    if (m_args_map.find(opt)->second == "true")
      return true;
    else if (m_args_map.find(opt)->second == "false")
      return false;
    else
      throw std::logic_error("Option is not a boolean: " + std::string(opt));
  }

  /*
   * @brief Get the string value of an option
   * @param opt Option name
   * @return String value of the option
   * @throw std::logic_error If the option does not exist
   */
  [[nodiscard]] std::string get_string(std::string_view opt) const {
    if (!has_opt(opt))
      throw std::logic_error("No such option: " + std::string(opt));

    return m_args_map.find(opt)->second;
  }

  /*
   * @brief Get the integer value of an option
   * @param opt Option name
   * @return Integer value of the option
   * @throw std::logic_error If the option does not exist or is not an integer
   */
  [[nodiscard]] long long get_int(std::string_view opt) const {
    if (!has_opt(opt))
      throw std::logic_error("No such option: " + std::string(opt));

    return std::stoll(m_args_map.find(opt)->second);
  }

  /*
   * @brief Get the double value of an option
   * @param opt Option name
   * @return Double value of the option
   * @throw std::logic_error If the option does not exist or is not a double
   */
  [[nodiscard]] long double get_double(std::string_view opt) const {
    if (!has_opt(opt))
      throw std::logic_error("No such option: " + std::string(opt));

    return std::stold(m_args_map.find(opt)->second);
  }

  /*
   * @brief Display the current state of the option maps
   */
  void show() const {
    if (m_opt_map.empty() && m_args_map.empty()) {
      std::cout << "empty argument\n";
      return;
    }

    if (!m_opt_map.empty()) {
      std::cout << "m_opt_map: \n";
      for (auto i = m_opt_map.begin(); i != m_opt_map.cend(); i++) {
        switch (i->second) {
        case OptionType::OPT_UNKNOWN:
          std::cout << i->first << " OPT_UNKNOWN\n";
          break;

        case OptionType::OPT_NO:
          std::cout << i->first << " OPT_NO\n";
          break;

        case OptionType::OPT_OPTIONAL:
          std::cout << i->first << " OPT_OPTIONAL\n";
          break;

        case OptionType::OPT_REQUIRED:
          std::cout << i->first << " OPT_REQUIRED\n";
          break;

        default:
          break;
        }
      }
    }

    if (!m_args_map.empty()) {
      std::cout << "m_args_map: \n";
      for (auto i = m_args_map.begin(); i != m_args_map.cend(); i++) {
        std::cout << i->first << ' ' << i->second << '\n';
      }
    }
  }

protected:
  /*
   * @brief Get the type of an option
   * @param opt Option name
   * @return Type of the option
   */
  OptionType get_type(std::string_view opt) const {
    auto itor = m_opt_map.find(opt);

    if (itor == m_opt_map.cend()) {
      return OptionType::OPT_UNKNOWN;
    }
    return itor->second;
  }

private:
  struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char *str) const { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const {
      return hash_type{}(str);
    }
    std::size_t operator()(const std::string &str) const {
      return hash_type{}(str);
    }
  };

  std::unordered_map<std::string, OptionType, string_hash, std::equal_to<>>
      m_opt_map; // Map to store option names and their types
  std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>
      m_args_map; // Map to store option names and their argument values
};

NAMESPACE_OPTION_END

#endif // OPTION_HPP
