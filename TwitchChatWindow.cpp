//
// Created by vcunningham on 5/7/19.
//

#include <iostream>
#include <pwd.h>
#include <fstream>
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
    std::string token_file_path;

    if ((token_file_path = getenv("HOME")).empty()) {
        token_file_path = getpwuid(getuid())->pw_dir;
    }

    token_file_path.append("/.local/share/twitch_chat_client");
    std::ifstream token_input_file(token_file_path + "/token");

    if(!token_input_file.fail()) {
        std::string token_string;

        token_input_file.seekg(0, std::ios::end);
        token_string.reserve(token_input_file.tellg());
        token_input_file.seekg(0, std::ios::beg);

        token_string.assign((std::istreambuf_iterator<char>(token_input_file)),
                            std::istreambuf_iterator<char>());
        m_Twitch->set_token(token_string);
        return;
    }

    m_RefBuilder->get_widget("login_dialog", m_LoginDialog);
    auto response = m_LoginDialog->run();
    m_LoginDialog->close();
    switch(response) {
        case(Gtk::RESPONSE_OK):
        {
            Gtk::Entry *oauth_entry;
            m_RefBuilder->get_widget("login_oauth", oauth_entry);
            Glib::ustring token = oauth_entry->get_text();
            if(token.empty()) {
                close();
                break;
            }

            struct stat st {};
            if(stat(token_file_path.c_str(), &st) == 0) {
                if ((st.st_mode & S_IFDIR) == 0) {
                    const int mkdir_error = mkdir(token_file_path.c_str(), S_IRWXU);
                    if (-1 == mkdir_error) {
                        std::cerr << "failed to create directory " << token_file_path.c_str() << std::endl;
                        exit(4);
                    }
                }
            }
            std::ofstream token_file;
            token_file.open(token_file_path + "/token");
            token_file << token << std::endl;
            token_file.close();
            std::cout << "file written to " << token_file_path << std::endl;

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
