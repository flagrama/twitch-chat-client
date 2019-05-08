//
// Created by vcunningham on 4/29/19.
//

#ifndef C_IRC_CLIENT_TWITCH_H
#define C_IRC_CLIENT_TWITCH_H

#include <netdb.h>
#include <thread>
#include <queue>

class Twitch {
public:
    Twitch();
    ~Twitch();

    void read_responses(std::queue<std::string> &text_queue);
    void set_token(std::string token_string);
    void disconnect();
private:
    struct addrinfo *addr;
    int sockfd;
    bool stopping;
    std::string token;

    static addrinfo *get_addrinfo();
    int get_socket();
    void connect();
    void login();
    static std::string message_display(const std::string &prefix, const std::string &trailing);
};


#endif //C_IRC_CLIENT_TWITCH_H
