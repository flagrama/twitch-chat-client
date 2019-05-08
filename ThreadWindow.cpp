//
// Created by vcunningham on 5/2/19.
//

#include "ThreadWindow.h"
#include <gtkmm/builder.h>

ThreadWindow::ThreadWindow() :
    m_Dispatcher(),
    m_RefBuilder(Gtk::Builder::create()) {}

void ThreadWindow::notify() {
    m_Dispatcher.emit();
}

void ThreadWindow::on_notification() {
    update_widgets();
}
