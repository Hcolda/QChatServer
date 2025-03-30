#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#include <format>
#include <stacktrace>
#include <source_location>
#include <string_view>
#include <string>
#include <filesystem>

#if defined(_MSC_VER) && defined(__cpp_lib_stacktrace) && defined(__cpp_lib_source_location) && defined(__cpp_lib_format)
    #define ERROR_WITH_STACKTRACE(errmsg) \
        [](std::string_view err, std::source_location location = std::source_location::current()) { \
            return std::format("error: {}\nin file \"{}\" line {}, function \"{}\"\nstack trace: \n{}\n", \
                err, \
                location.file_name(), \
                location.line(), \
                location.function_name(), \
                std::to_string(std::stacktrace::current())); \
            }(errmsg)
#else
    #define ERROR_WITH_STACKTRACE(errmsg) \
        std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), \
            __LINE__)
#endif // __cpp_lib_stacktrace

#include <functional>
#include <cstddef>

#include "groupid.hpp"
#include "userid.hpp"

namespace qls
{

enum class DeviceType
{
    Unknown = 0,
    PersonalComputer,
    Phone,
    Web
};

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
};

struct PrivateRoomIDStruct
{
    UserID user_id_1;
    UserID user_id_2;
    
    friend bool operator==(const PrivateRoomIDStruct& a, const PrivateRoomIDStruct& b)
    {
        return (a.user_id_1 == b.user_id_1 && a.user_id_2 == b.user_id_2) ||
            (a.user_id_2 == b.user_id_1 && a.user_id_1 == b.user_id_2);
    }

    friend bool operator!=(const PrivateRoomIDStruct& a, const PrivateRoomIDStruct& b)
    {
        return !(a == b);
    }
};

class PrivateRoomIDStructHasher
{
public:
    PrivateRoomIDStructHasher() = default;
    ~PrivateRoomIDStructHasher() = default;

    template<class T, class Y =
        std::enable_if_t<std::is_same_v<std::decay_t<T>, PrivateRoomIDStruct>>>
    std::size_t operator()(T&& s) const
    {
        std::hash<long long> hasher;
        return hasher(s.user_id_1.getOriginValue()) ^ hasher(s.user_id_2.getOriginValue());
    }
};

struct GroupVerificationStruct
{
    GroupID group_id;
    UserID user_id;

    friend bool operator==(const GroupVerificationStruct& a, const GroupVerificationStruct& b)
    {
        return a.group_id == b.group_id && a.user_id == b.user_id;
    }

    friend bool operator!=(const GroupVerificationStruct& a, const GroupVerificationStruct& b)
    {
        return !(a == b);
    }
};

class GroupVerificationStructHasher
{
public:
    GroupVerificationStructHasher() = default;
    ~GroupVerificationStructHasher() = default;

    template<class T, class Y =
        std::enable_if_t<std::is_same_v<std::decay_t<T>, GroupVerificationStruct>>>
    std::size_t operator()(T&& g) const
    {
        std::hash<long long> hasher;
        return hasher(g.group_id.getOriginValue()) ^ hasher(g.user_id.getOriginValue());
    }
};

} // namespace qls

#endif // !DEFINITION_HPP
