//
// Created by vcunningham on 5/2/19.
//

#ifndef TWITCH_CHAT_THREADWINDOW_H
#define TWITCH_CHAT_THREADWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/builder.h>
#include <glibmm/dispatcher.h>

class ThreadWindow : public Gtk::Window {
public:
    ThreadWindow();
    void notify();

protected:
    virtual void update_widgets() = 0;
    virtual void on_notification();

    Glib::RefPtr<Gtk::Builder> m_RefBuilder;
    Glib::Dispatcher m_Dispatcher;
};

#endif //TWITCH_CHAT_THREADWINDOW_H
