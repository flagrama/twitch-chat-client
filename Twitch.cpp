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
#include "Twitch.h"

Twitch::Twitch() {
    addr = get_addrinfo();
    sockfd = get_socket();
    stopping = false;
}

Twitch::~Twitch() {
    freeaddrinfo(addr);
    close(sockfd);
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
    int socket = ::socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol);

    if (socket == -1) {
        fprintf(stderr, "socket error: %d\n", errno);
        exit(errno);
    }

    return socket;
}

void Twitch::connect() {
    int connection = ::connect(this->sockfd, this->addr->ai_addr, this->addr->ai_addrlen);

    if (connection == -1) {
        fprintf(stderr, "connect error: %d\n", errno);
        exit(errno);
    }
}

void Twitch::set_token(std::string token_string) {
    this->token = std::move(token_string);
}

void Twitch::login() {
    std::string message;
    message = "PASS " + token + '\n';
    send(this->sockfd, message.c_str(), message.length(), 0);

    message = "NICK com.flagrama.*-twitch-chat\n";
    send(this->sockfd, message.c_str(), message.length(), 0);
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
        int bytes_received = recv(this->sockfd, response_buffer, sizeof response_buffer, 0);
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
            int prefix_pos = message.find(':') + 1;
            int command_pos = message.find(' ', prefix_pos) + 1;
            int params_pos = message.find(' ', command_pos) + 1;
            int trailing_pos = message.find(':', prefix_pos) + 1;

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
                std::string reply = "PONG " + server;
                send(this->sockfd, reply.c_str(), reply.length(), 0);
            }
            else if (message.substr(0, server.length()) == server) {
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
                        std::cout << "Bad OAuth token" << std::endl;
                    }
                    if (trailing == "Improperly formatted auth") {
                        std::cout << "Not an OAuth token" << std::endl;
                    }
                }
                strcat(message_buffer, message_display(prefix, trailing).c_str());
                text_queue.push(message_buffer);
            }
            if(command == "PRIVMSG") {
                strcat(message_buffer, message_display(prefix, trailing).c_str());
                text_queue.push(message_buffer);
            }

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
