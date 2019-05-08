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

    message = "CAP REQ :twitch.tv/tags\n";
    send(this->sockfd, message.c_str(), message.length(), 0);

    message = "JOIN #flagrama\n";
    send(this->sockfd, message.c_str(), message.length(), 0);
}

void Twitch::disconnect() {
    stopping = true;
}

void Twitch::read_responses(std::queue<std::string> &text_queue) {
    std::string server;
    char message_buffer[1024] = "";

    text_queue.push("Connecting to Twitch.\n");
    connect();
    text_queue.push("Connected. Now logging in.\n");
    login();

    memset(message_buffer, 0, sizeof message_buffer);

    while(!stopping) {
        int bytes_received = recv(this->sockfd, message_buffer, sizeof message_buffer, 0);
        std::string response(message_buffer, 0, bytes_received);

        if(bytes_received == 0) return;

        if(server.empty()) {
            int pos = response.find(':');
            int length = response.find(' ') - pos;
            server = response.substr(pos, length);
        }

        if (response.substr(0, 4) == "PING") {
            std::string reply = "PONG " + server;
            send(this->sockfd, reply.c_str(), reply.length(), 0);
        }
        else if (response.substr(0, server.length()) == server) {
            std::istringstream iss(response);
            std::string line;
            char buffer[1024] = "";

            while(std::getline(iss, line, '\n')) {
                std::string server_message = line.substr(line.find(':', 1) + 1, std::string::npos);
                strcat(buffer, server_message.c_str());
            }
            text_queue.push(buffer);
        }
        else {
            text_queue.push(response);
        }

        memset(message_buffer, 0, sizeof message_buffer);
    }
}
