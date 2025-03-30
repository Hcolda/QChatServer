#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#include <format>
#include <filesystem>
#include <cstddef>

#if _HAS_CXX23
// c++23 stacktrace
#include <stacktrace>
#endif

#if _HAS_CXX23
#define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
#else
#define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
#endif

#endif // !DEFINITION_HPP