cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(Server)

find_package(OpenSSL REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-mariadb-connector-cpp CONFIG REQUIRED)

if(MINGW)
  find_library(WSOCK32_LIBRARY wsock32)
  find_library(WS2_32_LIBRARY ws2_32)
endif()

add_executable(Server
    main/main.cpp
    initialization/init.cpp
    jsonMessageProcess/JsonMsgProcess.cpp
    jsonMessageProcess/JsonMsgProcessCommand.cpp
    manager/manager.cpp
    manager/dataManager.cpp
    manager/verificationManager.cpp
    network/network.cpp
    room/room.cpp
    room/groupRoom/groupRoom.cpp
    room/groupRoom/groupRoomVerification.cpp
    room/privateRoom/privateRoom.cpp
    room/privateRoom/friendRoomVerification.cpp
    socketFunctions/socketFunctions.cpp
    user/user.cpp
    room/groupRoom/groupPermission.cpp
    input/input.cpp
    input/inputCommands.cpp)

target_include_directories(Server PUBLIC
    main
    include
    initialization
    jsonMessageProcess
    manager
    network
    room/groupRoom
    room/privateRoom
    room
    socketFunctions
    user
    input
    error)

target_link_libraries(Server PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    unofficial::mariadb-connector-cpp::mariadbcpp
    asio::asio
    Utils)

if(MINGW)
  target_link_libraries(Server PRIVATE wsock32 ws2_32)
endif()
