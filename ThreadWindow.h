//
// Created by vcunningham on 5/2/19.
//

#ifndef TWITCH_CHAT_THREADWINDOW_H
#define TWITCH_CHAT_THREADWINDOW_H

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>
#include <glibmm/dispatcher.h>
#include <thread>
#include "IrcWorker.h"

class ThreadWindow : public Gtk::Window {
public:
    ThreadWindow();
    void notify();

private:
    void update_widgets();
    void on_notification();

    Twitch* m_Twitch;

    Gtk::ScrolledWindow* m_Container;
    Gtk::TextView* m_TextView;

    Glib::Dispatcher m_Dispatcher;
    IrcWorker m_Irc;
    std::thread* m_IrcThread;
};


#endif //TWITCH_CHAT_THREADWINDOW_H
