//
// Created by vcunningham on 4/29/19.
//

#ifndef C_IRC_CLIENT_MAIN_H
#define C_IRC_CLIENT_MAIN_H

#include <netdb.h>

addrinfo *get_twitch_addrinfo();
int get_twitch_socket(addrinfo *addrinfo);
void twitch_connect(int sockfd, addrinfo *addrinfo);
void twitch_login(int sockfd);
void twitch_responses(int sockfd);

#endif //C_IRC_CLIENT_MAIN_H
