#ifndef STRING_PARAM_HPP
#define STRING_PARAM_HPP

#include <cassert>
#include <format>
#include <iterator>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace qls {

template <typename Alloc> class basic_string_param {
public:
  basic_string_param(std::string_view view) : is_owned_(false), view_(view) {}

  basic_string_param(
      const std::basic_string<char, std::char_traits<char>, Alloc> &str)
      : is_owned_(false), view_(str) {}

  basic_string_param(
      std::basic_string<char, std::char_traits<char>, Alloc> &&str) noexcept
      : is_owned_(true), buffer_(std::move(str)) {}

  basic_string_param(const basic_string_param &) = delete;
  basic_string_param &operator=(const basic_string_param &) = delete;

  basic_string_param(basic_string_param &&str) noexcept
      : is_owned_(str.is_owned_), view_(str.view_),
        buffer_(std::move(str.buffer_)) {}

  basic_string_param &operator=(basic_string_param &&str) noexcept {
    if (this != &str) {
      is_owned_ = str.is_owned_;
      view_ = str.view_;
      buffer_ = std::move(str.buffer_);
    }
    return *this;
  }

  std::size_t size() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return s.size(); }, buffer_);
    }
    return view_.size();
  }

  operator std::string_view() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return std::string_view(s); },
                        buffer_);
    }
    return view_;
  }

  bool is_owned() const { return is_owned_; }

  bool is_pmr() const {
    if (is_owned_) {
      return std::holds_alternative<
          std::basic_string<char, std::char_traits<char>, Alloc>>(buffer_);
    }
    return false;
  }

  std::basic_string<char, std::char_traits<char>, Alloc> extract() && {
    if (is_owned_) {
      return std::get<std::basic_string<char, std::char_traits<char>, Alloc>>(
          std::move(buffer_));
    }
    throw std::logic_error("Cannot extract from non-owned string_param");
  }

  friend bool operator==(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) == std::string_view(rhs);
  }

  friend bool operator<(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) < std::string_view(rhs);
  }

  friend bool operator!=(const string_param &lhs, const string_param &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<=(const string_param &lhs, const string_param &rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const string_param &lhs, const string_param &rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const string_param &lhs, const string_param &rhs) {
    return !(lhs < rhs);
  }

private:
  std::string_view view_;
  std::basic_string<char, std::char_traits<char>, Alloc> buffer_;
  bool is_owned_;
};

using stdstring_param = basic_string_param<std::allocator<char>>;
using pmrstring_param =
    basic_string_param<std::pmr::polymorphic_allocator<char>>;

class string_param {
public:
  string_param(std::string_view view) : is_owned_(false), view_(view) {}

  string_param(const std::string &str) : is_owned_(false), view_(str) {}

  string_param(std::string &&str) noexcept
      : is_owned_(true), buffer_(std::move(str)) {}

  string_param(const std::pmr::string &str) : is_owned_(false), view_(str) {}

  string_param(std::pmr::string &&str) noexcept
      : is_owned_(true), buffer_(std::move(str)) {}

  template <typename It, std::sentinel_for<It> S>
  string_param(It first, S last) : is_owned_(false), view_(first, last) {}

  template <std::size_t N>
  string_param(const char (&str)[N], std::size_t size = N - 1)
      : is_owned_(false), view_(str, size) {
    assert(size < N);
  }

  string_param(const char *str) {
    if (str) {
      is_owned_ = false;
      view_ = std::string_view(str);
    } else {
      throw std::invalid_argument("Null pointer passed to string_param");
    }
  }

  string_param(const string_param &) = delete;
  string_param &operator=(const string_param &) = delete;

  string_param(string_param &&str) noexcept
      : is_owned_(str.is_owned_), view_(str.view_),
        buffer_(std::move(str.buffer_)) {}

  string_param &operator=(string_param &&str) noexcept {
    if (this != &str) {
      is_owned_ = str.is_owned_;
      view_ = str.view_;
      buffer_ = std::move(str.buffer_);
    }
    return *this;
  }

  std::size_t size() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return s.size(); }, buffer_);
    }
    return view_.size();
  }

  operator std::string_view() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return std::string_view(s); },
                        buffer_);
    }
    return view_;
  }

  bool is_owned() const { return is_owned_; }

  bool is_std() const {
    if (is_owned_) {
      return std::holds_alternative<std::string>(buffer_);
    }
    return false;
  }

  bool is_pmr() const {
    if (is_owned_) {
      return std::holds_alternative<std::pmr::string>(buffer_);
    }
    return false;
  }

  template <typename T,
            typename = std::enable_if_t<std::is_same_v<T, std::string> ||
                                        std::is_same_v<T, std::pmr::string>>>
  T extract() && {
    if (is_owned_) {
      return std::get<T>(std::move(buffer_));
    }
    throw std::logic_error("Cannot extract from non-owned string_param");
  }

  std::string extract_std() && {
    return std::move(*this).extract<std::string>();
  }

  std::pmr::string extract_pmr() && {
    return std::move(*this).extract<std::pmr::string>();
  }

  friend bool operator==(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) == std::string_view(rhs);
  }

  friend bool operator<(const string_param &lhs, const string_param &rhs) {
    return std::string_view(lhs) < std::string_view(rhs);
  }

  friend bool operator!=(const string_param &lhs, const string_param &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<=(const string_param &lhs, const string_param &rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const string_param &lhs, const string_param &rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const string_param &lhs, const string_param &rhs) {
    return !(lhs < rhs);
  }

private:
  std::string_view view_;
  std::variant<std::string, std::pmr::string> buffer_;
  bool is_owned_;
};

} // namespace qls

namespace std {
template <> struct formatter<qls::string_param> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const qls::string_param &str_param, FormatContext &ctx) const {
    return format_to(ctx.out(), "{}", std::string_view(str_param));
  }
};
} // namespace std

#endif // !STRING_PARAM_HPP
