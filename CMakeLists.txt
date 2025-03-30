cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(QingLiaoChatServer)

set(BUILD_TEST_CLIENT ON)

add_subdirectory(utils)
add_subdirectory(server)
if (BUILD_TEST_CLIENT)
  add_subdirectory(testclient)
endif()
