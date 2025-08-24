#ifndef STRING_PARAM_HPP
#define STRING_PARAM_HPP

#include <cassert>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

namespace qls {

template <typename Alloc> class base_string_param {
public:
  base_string_param(std::string_view view) : is_owned_(false), view_(view) {}

  base_string_param(
      const std::basic_string<char, std::char_traits<char>, Alloc> &str)
      : is_owned_(false), view_(str) {}

  base_string_param(
      std::basic_string<char, std::char_traits<char>, Alloc> &&str)
      : is_owned_(true), buffer_(str) {}

  base_string_param(const base_string_param &) = delete;
  base_string_param &operator=(const base_string_param &) = delete;
  base_string_param(base_string_param &&) = default;
  base_string_param &operator=(base_string_param &&) = default;

  std::size_t size() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return s.size(); }, buffer_);
    } else {
      return view_.size();
    }
  }

  operator std::string_view() const {
    if (is_owned_) {
      return std::visit([](const auto &s) { return std::string_view(s); },
                        buffer_);
    } else {
      return view_;
    }
  }

  bool is_owned() const { return is_owned_; }

  bool is_pmr() const {
    if (is_owned_) {
      return std::holds_alternative<
          std::basic_string<char, std::char_traits<char>, Alloc>>(buffer_);
    } else {
      return false;
    }
  }

  std::basic_string<char, std::char_traits<char>, Alloc> extract() && {
    if (is_owned_) {
      return std::visit([](auto &&s) { return std::move(s); },
                        std::move(buffer_));
    } else {
      throw std::logic_error("Cannot extract from non-owned string_param");
    }
  }

private:
  std::string_view view_;
  std::basic_string<char, std::char_traits<char>, Alloc> buffer_;
  bool is_owned_;
};

class string_param {
public:
  string_param(std::string_view view) : is_owned_(false), view_(view) {}

  string_param(const std::string &str) : is_owned_(false), view_(str) {}

  string_param(std::string &&str) : is_owned_(true), buffer_(std::move(str)) {}

  string_param(const std::pmr::string &str) : is_owned_(false), view_(str) {}

  string_param(std::pmr::string &&str)
      : is_owned_(true), buffer_(std::move(str)) {}

  template <std::size_t N>
  string_param(const char (&str)[N], std::size_t size = N - 1)
      : is_owned_(false), view_(str, size) {
    assert(size < N);
  }

  string_param(const string_param &str)
      : is_owned_(str.is_owned_), view_(str.view_), buffer_(str.buffer_) {}

  string_param &operator=(const string_param &str) {
    if (this != &str) {
      is_owned_ = str.is_owned_;
      view_ = str.view_;
      buffer_ = str.buffer_;
    }
    return *this;
  }

  string_param(string_param &&str)
      : is_owned_(str.is_owned_), view_(str.view_),
        buffer_(std::move(str.buffer_)) {}

  string_param &operator=(string_param &&str) {
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
      return std::holds_alternative<std::pmr::string>(buffer_);
    }
    return false;
  }

  std::string extract() && {
    if (is_owned_) {
      return std::get<std::string>(std::move(buffer_));
    }
    throw std::logic_error("Cannot extract from non-owned string_param");
  }

  std::pmr::string extract_pmr() && {
    if (is_owned_) {
      return std::get<std::pmr::string>(std::move(buffer_));
    }
    throw std::logic_error("Cannot extract from non-owned string_param");
  }

private:
  std::string_view view_;
  std::variant<std::string, std::pmr::string> buffer_;
  bool is_owned_;
};

} // namespace qls

#endif // !STRING_PARAM_HPP
