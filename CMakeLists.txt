cmake_minimum_required(VERSION 3.14)
project(c_irc_client)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(c_irc_client main.cpp main.h Twitch.cpp Twitch.h)
target_link_libraries(c_irc_client ${CMAKE_THREAD_LIBS_INIT} ${GTK3_LIBRARIES})