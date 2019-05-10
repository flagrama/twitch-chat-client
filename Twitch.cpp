//
// Created by vcunningham on 4/29/19.
//

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <utility>
#include <chrono>
#include <pwd.h>
#include <fstream>
#include <sys/stat.h>
#include "Twitch.h"

Twitch::Twitch() {
    m_Addr = get_addrinfo();
    m_SockFd = get_socket();
    stopping = false;
}

Twitch::~Twitch() {
    freeaddrinfo(m_Addr);
    close(m_SockFd);
}

bool Twitch::get_token() {
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
        set_token(token_string, false);
        return true;
    }
    return false;
}

void Twitch::set_token(std::string token_string, bool saving) {
    this->token = std::move(token_string);
    std::string token_file_path;
    std::ofstream token_file;

    if(saving) {
        std::cout << "saving" << std::endl;
        if ((token_file_path = getenv("HOME")).empty()) {
            token_file_path = getpwuid(getuid())->pw_dir;
        }

        token_file_path.append("/.local/share/twitch_chat_client");

        struct stat st {};
        if(stat(token_file_path.c_str(), &st) != 0) {
            if ((st.st_mode & S_IFDIR) == 0) {
                const int mkdir_error = mkdir(token_file_path.c_str(), S_IRWXU);
                if (-1 == mkdir_error) {
                    std::cerr << "failed to create directory " << token_file_path.c_str() << std::endl;
                    exit(4);
                }
            }
        }

        token_file_path.append("/token");

        token_file.open(token_file_path);
        if(token_file.fail()) {
            std::cerr << "failed to open file" ;
            exit(5);
        }
        token_file << token << std::endl;
        token_file.close();
    }
}

void Twitch::delete_token() {
    std::string token_file;

    if ((token_file = getenv("HOME")).empty()) {
        token_file = getpwuid(getuid())->pw_dir;
    }

    token_file.append("/.local/share/twitch_chat_client/token");
    remove(token_file.c_str());
}

addrinfo *Twitch::get_addrinfo() {
    int status;
    struct addrinfo hints {};
    struct addrinfo *result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo("irc.chat.twitch.tv", "6667", &hints, &result);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    return result;
}

int Twitch::get_socket() {
    int socket = ::socket(this->m_Addr->ai_family, this->m_Addr->ai_socktype, this->m_Addr->ai_protocol);

    if (socket == -1) {
        fprintf(stderr, "socket error: %d\n", errno);
        exit(errno);
    }

    return socket;
}

void Twitch::connect() {
    int connection = ::connect(this->m_SockFd, this->m_Addr->ai_addr, this->m_Addr->ai_addrlen);

    if (connection == -1) {
        fprintf(stderr, "connect error: %d\n", errno);
        exit(errno);
    }
}

void Twitch::login() {
    std::string message;
    message = "PASS " + token + '\n';
    send(this->m_SockFd, message.c_str(), message.length(), 0);

    message = "NICK com.flagrama.*-twitch-chat\n";
    send(this->m_SockFd, message.c_str(), message.length(), 0);

    message = "CAP REQ :twitch.tv/tags\n";
    send(this->m_SockFd, message.c_str(), message.length(), 0);

    message = "JOIN #zfg1\n";
    send(this->m_SockFd, message.c_str(), message.length(), 0);
}

void Twitch::disconnect() {
    stopping = true;
}

void Twitch::read_responses(std::queue<std::string> &text_queue) {
    std::string server;

    text_queue.push("Connecting to Twitch.\n");
    connect();
    text_queue.push("Connected. Now logging in.\n");
    login();

    while(!stopping) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        char response_buffer[1024] = "";
        int bytes_received = recv(this->m_SockFd, response_buffer, sizeof response_buffer, 0);
        std::string response(response_buffer, 0, bytes_received);

        if (bytes_received == 0) return;

        if (server.empty()) {
            int pos = response.find(':');
            int length = response.find(' ') - pos;
            server = response.substr(pos, length);
        }

        std::istringstream iss(response);
        std::string message;
        char message_buffer[1024] = "";

        while (std::getline(iss, message, '\n')) {
            int prefix_pos;
            if(message[0] == '@') {
                prefix_pos = message.find(' ') + 2;
            }
            else {
                prefix_pos = message.find(':') + 1;
            }
            int command_pos = message.find(' ', prefix_pos) + 1;
            int params_pos = message.find(' ', command_pos) + 1;
            int trailing_pos = message.find(" :", prefix_pos) + 2;

            std::string prefix = message.substr(prefix_pos, command_pos - prefix_pos - 1);
            if(prefix.find(".tmi.twitch.tv") != std::string::npos) {
                int username_pos;
                if(message.find('@', prefix_pos) != std::string::npos) {
                    username_pos = message.find('@', prefix_pos) + 1;
                }
                else {
                    username_pos = message.find(':') + 1;
                }
                int twitch_pos = message.find(".tmi.twitch.tv");
                prefix = message.substr(username_pos, twitch_pos - username_pos);
            }

            std::string command = message.substr(command_pos, params_pos - command_pos - 1);

            std::string params = message.substr(params_pos, trailing_pos - params_pos - 1);

            std::string trailing = message.substr(trailing_pos, message.length() - trailing_pos - 1);

            if (message.substr(0, 4) == "PING") {
                std::string reply = "PONG " + server + '\n';
                send(this->m_SockFd, reply.c_str(), reply.length(), 0);
            }
            else if (message.substr(0, server.length()) == server) {
                if (command == "421") {
                    strcat(message_buffer, ("Unknown Command: " + params).c_str());
                    text_queue.push(message_buffer);
                }

                if (command == "CAP" && params.find("* ACK") != std::string::npos) {
                    std::string capabilities;
                    if (trailing.find("/tags") != std::string::npos) {
                        capabilities.append("TAGS ");
                    }
                    if (trailing.find("/membership") != std::string::npos) {
                        capabilities.append("MEMBERSHIP ");
                    }
                    if (trailing.find("/commands") != std::string::npos) {
                        capabilities.append("COMMANDS ");
                    }

                    strcat(message_buffer, ("Capability " + capabilities + "Acknowledged\n").c_str());
                    text_queue.push(message_buffer);
                    continue;
                }

                if (command == "NOTICE") {
                    if (trailing == "Login authentication failed") {
                        std::cerr << "Bad OAuth token" << std::endl;
                        delete_token();
                        exit(5);
                    }
                    if (trailing == "Improperly formatted auth") {
                        std::cerr << "Not an OAuth token" << std::endl;
                        delete_token();
                        exit(6);
                    }
                }
                strcat(message_buffer, message_display(prefix, trailing).c_str());
                text_queue.push(message_buffer);
            }
            if(command == "PRIVMSG") {
                strcat(message_buffer, message_display(prefix, trailing).c_str());
                text_queue.push(message_buffer);
            }

            std::cout << std::endl;
            std::cout << "message: " << message << std::endl;
            std::cout << "prefix: " << prefix << std::endl;
            std::cout << "command: " << command << std::endl;
            std::cout << "params: " << params << std::endl;
            std::cout << "trailing: " << trailing << std::endl;

            memset(message_buffer, 0, sizeof message_buffer);
            memset(response_buffer, 0, sizeof response_buffer);
        }
    }
}

std::string Twitch::message_display(const std::string &prefix, const std::string &trailing) {
    std::string result;
    result.append("<");
    result.append(prefix);
    result.append("> ");
    result.append(trailing);
    result.append("\n");
    return result;
}
