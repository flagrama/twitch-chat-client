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
#include "Twitch.h"

Twitch::Twitch() {
    addr = get_addrinfo();
    sockfd = get_socket();
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

void Twitch::login() {
    std::string message;
    std::string token;

    token = std::getenv("TOKEN");
    message = "PASS " + token + '\n';
    send(this->sockfd, message.c_str(), message.length(), 0);

    message = "NICK com.flagrama.*-twitch-chat\n";
    send(this->sockfd, message.c_str(), message.length(), 0);
}

void Twitch::read_responses(char* text_buffer) {
    std::string server;
    char message_buffer[1024];

    strcat(text_buffer, "Connecting to Twitch.\n");
    connect();
    strcat(text_buffer, "Connected. Now logging in.\n");
    login();

    memset(message_buffer, 0, sizeof message_buffer);

    while(true) {
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
            char buffer[1024];

            while(std::getline(iss, line, '\n')) {
                std::string server_message = line.substr(line.find(':', 1) + 1, std::string::npos);
                strcat(buffer, server_message.c_str());
            }
            strcat(text_buffer, buffer);
        }
        else {
            strcat(text_buffer, response.c_str());
        }

        memset(message_buffer, 0, sizeof message_buffer);
    }
}
