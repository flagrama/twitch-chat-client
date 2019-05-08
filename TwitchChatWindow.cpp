//
// Created by vcunningham on 5/7/19.
//

#include <iostream>
#include "TwitchChatWindow.h"

TwitchChatWindow::TwitchChatWindow() :
    m_Container(),
    m_TextView(),
    m_LoginDialog(),
    m_Twitch(new Twitch()),
    m_Irc(m_Twitch),
    m_IrcThread(nullptr)
{
    setup_window();
    display_login_dialog();
    start_irc_thread();
}

void TwitchChatWindow::setup_window() {
    set_title("Twitch Chat");
    set_default_size(640, 480);

    try {
        m_RefBuilder->add_from_file("twitch-chat.glade");
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
    m_RefBuilder->get_widget("container", m_Container);
    m_RefBuilder->get_widget("irc_messages", m_TextView);
    add(*m_Container);
    show();
}

void TwitchChatWindow::display_login_dialog() {
    m_RefBuilder->get_widget("login_dialog", m_LoginDialog);
    auto response = m_LoginDialog->run();
    m_LoginDialog->close();
    switch(response) {
        case(Gtk::RESPONSE_OK):
        {
            Gtk::Entry *oauth_entry;
            m_RefBuilder->get_widget("login_oauth", oauth_entry);
            Glib::ustring token = oauth_entry->get_text();
            m_Twitch->set_token(std::string(token));
            break;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
            close();
            break;
        }
        default:
        {
            close();
            break;
        }
    }
}

void TwitchChatWindow::start_irc_thread() {
    m_Dispatcher.connect(sigc::mem_fun(*this, &TwitchChatWindow::on_notification));

    auto buffer = m_TextView->get_buffer();
    buffer->create_mark("last_line", buffer->end(), true);

    m_IrcThread = new std::thread([this] {
        m_Irc.process_responses(this);
    });
}

void TwitchChatWindow::update_widgets() {
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

void TwitchChatWindow::on_notification() {
    if(m_IrcThread && m_Irc.has_stopped()) {
        if(m_IrcThread->joinable()) m_IrcThread->join();
        delete m_IrcThread;
        m_IrcThread = nullptr;
    }

    ThreadWindow::on_notification();
}
