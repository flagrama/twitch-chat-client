#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <stdio.h>
#include <future>
#include <unistd.h>

#include "main.h"

int main(int argc, char *argv[]) {
    struct addrinfo *twitch_addr;
    int twitch_sockfd;

    twitch_addr = get_twitch_addrinfo();
    twitch_sockfd = get_twitch_socket(twitch_addr);
    twitch_connect(twitch_sockfd, twitch_addr);
    std::future<void> future = std::async (twitch_responses, twitch_sockfd);
    twitch_login(twitch_sockfd);

    future.get();

    std::cout << "Goodbye, World!" << std::endl;

    freeaddrinfo(twitch_addr);
    close(twitch_sockfd);

    return 0;
}

addrinfo *get_twitch_addrinfo() {
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

int get_twitch_socket(addrinfo *addrinfo) {
    int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);

    if (sockfd == -1) {
        fprintf(stderr, "socket error: %d\n", errno);
        exit(errno);
    }

    return sockfd;
}

void twitch_connect(int sockfd, addrinfo *addrinfo) {
    int connection = connect(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen);

    if (connection == -1) {
        fprintf(stderr, "connect error: %d\n", errno);
        exit(errno);
    }
}

void twitch_login(int sockfd) {
    std::string message;
    char buffer[256];

    snprintf(buffer, sizeof buffer, "PASS %s\n", std::getenv("TOKEN"));
    message = buffer;
    send(sockfd, message.c_str(), message.length(), 0);
    memset(buffer, 0, sizeof buffer);

    snprintf(buffer, sizeof buffer, "NICK %s\n", std::getenv("USERNICK"));
    message = buffer;
    send(sockfd, message.c_str(), message.length(), 0);
    memset(buffer, 0, sizeof buffer);
}

void twitch_responses(int sockfd) {
    std::cout << "Waiting for server responses..." << std::endl;
    while(true) {
        char buffer[4096];

        int bytes_received = recv(sockfd, buffer, sizeof buffer, 0);
        if(bytes_received == 0) return;
        if(bytes_received > 0) {
            std::string response(buffer, 0, bytes_received);
            std::cout << response;
            if (response.substr(0, 4) == "PING") {
                std::string reply;
                snprintf(buffer, sizeof buffer, "PONG %s", response.substr(5, std::string::npos).c_str());
                reply = buffer;
                send(sockfd, reply.c_str(), reply.length(), 0);
                std::cout << reply;
            }
        }

        memset(buffer, 0, sizeof buffer);
    }
}
