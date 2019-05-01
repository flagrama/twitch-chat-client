//
// Created by vcunningham on 4/29/19.
//

#ifndef C_IRC_CLIENT_TWITCH_H
#define C_IRC_CLIENT_TWITCH_H

#include <netdb.h>

class Twitch {
public:
    Twitch();
    ~Twitch();

    void read_responses(char* text_buffer);
private:
    struct addrinfo *addr;
    int sockfd;

    static addrinfo *get_addrinfo();
    int get_socket();
    void connect();
    void login();
};


#endif //C_IRC_CLIENT_TWITCH_H
