//
// Created by vcunningham on 4/29/19.
//

#ifndef C_IRC_CLIENT_TWITCH_H
#define C_IRC_CLIENT_TWITCH_H

#include <netdb.h>
#include <queue>

class Twitch {
public:
    Twitch();
    ~Twitch();

    void read_responses(std::queue<std::string> &text_queue);
    void disconnect();
private:
    struct addrinfo *addr;
    int sockfd;
    bool stopping;

    static addrinfo *get_addrinfo();
    int get_socket();
    void connect();
    void login();
};


#endif //C_IRC_CLIENT_TWITCH_H
