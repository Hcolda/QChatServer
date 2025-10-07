#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#include <cstdint>
#include <filesystem>
#include <format>
#include <source_location>
#include <stacktrace>
#include <string>
#include <string_view>

#if defined(_MSC_VER) && defined(__cpp_lib_stacktrace) &&                      \
    defined(__cpp_lib_source_location) && defined(__cpp_lib_format)
#define ERROR_WITH_STACKTRACE(errmsg)                                          \
  [](std::string_view err,                                                     \
     std::source_location location = std::source_location::current()) {        \
    return std::format("error: {}\nin file \"{}\" line {}, function "          \
                       "\"{}\"\nstack trace: \n{}\n",                          \
                       std::string_view(err), location.file_name(),            \
                       location.line(), location.function_name(),              \
                       std::to_string(std::stacktrace::current()));            \
  }(errmsg)
#else
#define ERROR_WITH_STACKTRACE(errmsg)                                          \
  std::format("error: {}\nin file \"{}\" line {}\n", errmsg,                   \
              std::filesystem::path(__FILE__).filename().string(), __LINE__)
#endif // __cpp_lib_stacktrace

#include <cstddef>
#include <functional>

#include "groupid.hpp"

#include "userid.hpp"

namespace qls {

enum class DeviceType : std::int8_t {
  Unknown = 0,
  PersonalComputer,
  Phone,
  Web
};

struct string_hash {
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  std::size_t operator()(const char *str) const { return hash_type{}(str); }
  std::size_t operator()(std::string_view str) const {
    return hash_type{}(str);
  }
};

struct PrivateRoomIDStruct {
  UserID user_id_1;
  UserID user_id_2;

  friend bool operator==(const PrivateRoomIDStruct &pri1,
                         const PrivateRoomIDStruct &pri2) {
    return (pri1.user_id_1 == pri2.user_id_1 &&
            pri1.user_id_2 == pri2.user_id_2) ||
           (pri1.user_id_2 == pri2.user_id_1 &&
            pri1.user_id_1 == pri2.user_id_2);
  }

  friend bool operator!=(const PrivateRoomIDStruct &pri1,
                         const PrivateRoomIDStruct &pri2) {
    return !(pri1 == pri2);
  }
};

class PrivateRoomIDStructHasher {
public:
  PrivateRoomIDStructHasher() = default;
  ~PrivateRoomIDStructHasher() = default;

  template <class T, class Y = std::enable_if_t<
                         std::is_same_v<std::decay_t<T>, PrivateRoomIDStruct>>>
  std::size_t operator()(T &&pri) const {
    std::hash<long long> hasher;
    return hasher(pri.user_id_1.getOriginValue()) ^
           hasher(pri.user_id_2.getOriginValue());
  }
};

struct GroupVerificationStruct {
  GroupID group_id;
  UserID user_id;

  friend bool operator==(const GroupVerificationStruct &gro1,
                         const GroupVerificationStruct &gro2) {
    return gro1.group_id == gro2.group_id && gro1.user_id == gro2.user_id;
  }

  friend bool operator!=(const GroupVerificationStruct &gro1,
                         const GroupVerificationStruct &gro2) {
    return !(gro1 == gro2);
  }
};

class GroupVerificationStructHasher {
public:
  GroupVerificationStructHasher() = default;
  ~GroupVerificationStructHasher() = default;

  template <class T, class Y = std::enable_if_t<std::is_same_v<
                         std::decay_t<T>, GroupVerificationStruct>>>
  std::size_t operator()(T &&gro) const {
    std::hash<long long> hasher;
    return hasher(gro.group_id.getOriginValue()) ^
           hasher(gro.user_id.getOriginValue());
  }
};

} // namespace qls

#endif // !DEFINITION_HPP
