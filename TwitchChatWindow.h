//
// Created by vcunningham on 5/7/19.
//

#ifndef TWITCH_CHAT_TWITCHCHATWINDOW_H
#define TWITCH_CHAT_TWITCHCHATWINDOW_H

#include <gtkmm.h>
#include <giomm.h>
#include "ThreadWindow.h"
#include "Twitch.h"
#include "IrcWorker.h"

class TwitchChatWindow : public ThreadWindow {
public:
    TwitchChatWindow();

protected:
    void update_widgets() final;
    void on_notification() final;

private:
    void setup_window();
    void display_login_dialog();
    void start_irc_thread();
    void connect_signals();
    void on_irc_message_box_activate (Gtk::Entry *irc_message_box);

    Gtk::Box* m_Container;
    Twitch* m_Twitch;
    Gtk::TextView* m_TextView;
    Gtk::Dialog* m_LoginDialog;
    IrcWorker m_Irc;
    std::thread* m_IrcThread;
};


#endif //TWITCH_CHAT_TWITCHCHATWINDOW_H
