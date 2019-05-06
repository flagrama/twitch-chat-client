//
// Created by vcunningham on 5/2/19.
//

#include "ThreadWindow.h"
#include "Twitch.h"
#include <iostream>
#include <gtkmm/builder.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>

ThreadWindow::ThreadWindow() :
    m_Container(),
    m_TextView(),
    m_Dispatcher(),
    m_Twitch(new Twitch()),
    m_Irc(m_Twitch),
    m_IrcThread(nullptr) {
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

    m_IrcThread = new std::thread([this] {
        m_Irc.process_responses(this);
    });
}

void ThreadWindow::update_widgets() {
    const int MAX_LINES = 500;
    Glib::ustring irc_message;
    m_Irc.get_data(&irc_message);

    auto buffer = m_TextView->get_buffer();
    auto iter = buffer->end();
    iter = buffer->insert(iter, irc_message);

    auto mark = buffer->get_mark("last_line");
    buffer->move_mark(mark, iter);

    auto excess_lines = buffer->get_line_count() - MAX_LINES;
    if(excess_lines > 0) {
        auto iter_start = buffer->get_iter_at_line(0);
        auto iter_end = buffer->get_iter_at_line(excess_lines);
        buffer->erase(iter_start, iter_end);
    }

    m_TextView->scroll_to(mark);
}

void ThreadWindow::notify() {
    m_Dispatcher.emit();
}

void ThreadWindow::on_notification() {
    if(m_IrcThread && m_Irc.has_stopped()) {
        if(m_IrcThread->joinable()) m_IrcThread->join();
        delete m_IrcThread;
        m_IrcThread = nullptr;
    }
    update_widgets();
}
