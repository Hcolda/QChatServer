#ifndef REGEX_MATCH_HPP
#define REGEX_MATCH_HPP

#include <regex>
#include <string_view>

namespace qls {

class RegexMatch {
public:
  RegexMatch() = default;
  ~RegexMatch() noexcept = default;

  [[nodiscard]] static bool emailMatch(std::string_view email) {
    static std::regex reg(R"((\w+\.)*\w+@(\w+\.)+[A-Za-z]+)",
                          std::regex::optimize);
    std::smatch results;
    return std::regex_match(email.cbegin(), email.cend(), reg);
  }

  [[nodiscard]] static bool ipAddressMatch(std::string_view ip_address) {
    static std::regex reg(
        R"((((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5])))",
        std::regex::optimize);
    return std::regex_match(ip_address.cbegin(), ip_address.cend(), reg);
  }

  [[nodiscard]] static bool phoneMatch(std::string_view phone) {
    static std::regex reg(R"(\d{11})", std::regex::optimize);
    return std::regex_match(phone.cbegin(), phone.cend(), reg);
  }
};
} // namespace qls

#endif // !REGEX_MATCH_HPP
