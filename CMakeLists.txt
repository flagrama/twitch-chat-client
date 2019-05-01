cmake_minimum_required(VERSION 3.14)
project(twitch-chat)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_custom_target(
        gui
        DEPENDS "${CMAKE_SOURCE_DIR}/twitch-chat.glade"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/twitch-chat.glade" "${CMAKE_CURRENT_BINARY_DIR}/twitch-chat.glade"
)

add_executable(twitch_chat main.cpp main.h Twitch.cpp Twitch.h)
add_dependencies(twitch_chat gui)
target_link_libraries(twitch_chat ${CMAKE_THREAD_LIBS_INIT} ${GTK3_LIBRARIES})