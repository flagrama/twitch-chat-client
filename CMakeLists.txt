cmake_minimum_required(VERSION 3.14)
project(c_irc_client)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads)

add_executable(c_irc_client main.cpp main.h)
target_link_libraries(c_irc_client ${CMAKE_THREAD_LIBS_INIT})