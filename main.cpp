#include "TwitchChatWindow.h"
#include <gtkmm/application.h>

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.flagrama.gtk-twitch-chat");
    TwitchChatWindow window;
    return app->run(window);
}
