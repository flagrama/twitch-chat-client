cmake_minimum_required(VERSION 3.14)
project(twitch-chat)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtkmm-3.0)
pkg_check_modules(GLIB REQUIRED glibmm-2.4)

include_directories(${GTK3_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_custom_target(
        gui
        DEPENDS "${CMAKE_SOURCE_DIR}/twitch-chat.glade"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/twitch-chat.glade" "${CMAKE_CURRENT_BINARY_DIR}/twitch-chat.glade"
)

add_executable(twitch_chat main.cpp Twitch.cpp Twitch.h ThreadWindow.cpp ThreadWindow.h IrcWorker.cpp IrcWorker.h TwitchChatWindow.cpp TwitchChatWindow.h)
add_dependencies(twitch_chat gui)
target_link_libraries(twitch_chat ${CMAKE_THREAD_LIBS_INIT} ${GTK3_LIBRARIES} ${GLIB_LIBRARIES})