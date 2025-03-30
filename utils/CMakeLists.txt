cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(Utils)

find_package(OpenSSL REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-mariadb-connector-cpp CONFIG REQUIRED)
find_package(kcp CONFIG REQUIRED)

if(MINGW)
    find_library(WSOCK32_LIBRARY wsock32)
    find_library(WS2_32_LIBRARY ws2_32)
endif()

add_library(Utils 
    network/dataPackage.cpp
    network/package.cpp
    error/qls_error.cpp
    parser/Ini.cpp
    parser/Json.cpp)
target_include_directories(Utils PUBLIC
    .
    network
    error
    parser
    crypto
    kcp/include)

target_link_libraries(Utils PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    unofficial::mariadb-connector-cpp::mariadbcpp
    asio::asio
    kcp::kcp)

if(MINGW)
    target_link_libraries(Utils PUBLIC wsock32 ws2_32)
endif()
