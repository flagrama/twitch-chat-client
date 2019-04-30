//
// Created by vcunningham on 4/29/19.
//

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "Twitch.h"

Twitch::Twitch() {
    addr = get_addrinfo();
    sockfd = get_socket();

    connect();

    std::thread responses (&Twitch::read_responses, this);

    login();
    responses.detach();
}

Twitch::~Twitch() {
    freeaddrinfo(addr);
    close(sockfd);
}

addrinfo *Twitch::get_addrinfo() {
    int status;
    struct addrinfo hints;
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
    int sockfd = socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol);

    if (sockfd == -1) {
        fprintf(stderr, "socket error: %d\n", errno);
        exit(errno);
    }

    return sockfd;
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
    char buffer[256];

    snprintf(buffer, sizeof buffer, "PASS %s\n", std::getenv("TOKEN"));
    message = buffer;
    send(this->sockfd, message.c_str(), message.length(), 0);
    memset(buffer, 0, sizeof buffer);

    snprintf(buffer, sizeof buffer, "NICK %s\n", std::getenv("USERNICK"));
    message = buffer;
    send(this->sockfd, message.c_str(), message.length(), 0);
    memset(buffer, 0, sizeof buffer);
}

void Twitch::read_responses() {
    std::cout << "Waiting for server responses..." << std::endl;
    while(true) {
        char buffer[4096];

        int bytes_received = recv(this->sockfd, buffer, sizeof buffer, 0);
        if(bytes_received == 0) return;
        if(bytes_received > 0) {
            std::string response(buffer, 0, bytes_received);
            std::cout << response;
            if (response.substr(0, 4) == "PING") {
                std::string reply;
                snprintf(buffer, sizeof buffer, "PONG %s", response.substr(5, std::string::npos).c_str());
                reply = buffer;
                send(this->sockfd, reply.c_str(), reply.length(), 0);
                std::cout << reply;
            }
        }

        memset(buffer, 0, sizeof buffer);
    }
}
