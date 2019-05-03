//
// Created by vcunningham on 5/2/19.
//

#include "ThreadWindow.h"
#include <iostream>
#include <gtkmm/builder.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

ThreadWindow::ThreadWindow() :
    m_Container(),
    m_TextView(),
    m_Dispatcher(),
    m_Worker(),
    m_WorkerThread(nullptr) {
    set_title("Twitch Chat");
    set_default_size(640, 480);
    set_can_focus(false);

    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_file("twitch-chat.glade");
    } catch(const Glib::FileError& ex) {
        std::cerr << "FileError: " << ex.what() << std::endl;
        exit(1);
    } catch(const Glib::MarkupError& ex) {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        exit(2);
    } catch(const Gtk::BuilderError& ex) {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        exit(3);
    }
    refBuilder->get_widget("container", m_Container);
    refBuilder->get_widget("irc_messages", m_TextView);
    add(*m_Container);

    m_Dispatcher.connect(sigc::mem_fun(*this, &ThreadWindow::on_notification));

    auto buffer = m_TextView->get_buffer();
    buffer->create_mark("last_line", buffer->end(), true);

    m_WorkerThread = new std::thread([this] {
        m_Worker.process_responses(this);
    });
}

void ThreadWindow::update_widgets() {
    Glib::ustring irc_message;
    m_Worker.get_data(&irc_message);

    auto buffer = m_TextView->get_buffer();
    Gtk::TextIter iter = buffer->end();
    iter = buffer->insert(iter, irc_message);

    auto mark = buffer->get_mark("last_line");
    buffer->move_mark(mark, iter);
    m_TextView->scroll_to(mark);
}

void ThreadWindow::notify() {
    m_Dispatcher.emit();
}

void ThreadWindow::on_notification() {
    if(m_WorkerThread && m_Worker.has_stopped()) {
        if(m_WorkerThread->joinable()) m_WorkerThread->join();
        delete m_WorkerThread;
        m_WorkerThread = nullptr;
    }
    update_widgets();
}
