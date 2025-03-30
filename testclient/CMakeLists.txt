cmake_minimum_required(VERSION 3.24)

project(TestClient)

find_package(OpenSSL REQUIRED)
find_package(asio CONFIG REQUIRED)

if(MINGW)
  find_library(WSOCK32_LIBRARY wsock32)
  find_library(WS2_32_LIBRARY ws2_32)
endif()

add_executable(TestClient
    main.cpp
    network.cpp
    session.cpp
    commands.cpp)
target_link_libraries(TestClient PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    asio::asio
    Utils)

if(MINGW)
  target_link_libraries(TestClient PRIVATE wsock32 ws2_32)
endif()
