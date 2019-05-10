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

    bool get_token();
    void read_responses(std::queue<std::string> &text_queue);
    void set_token(std::string token_string, bool saving);
    void disconnect();
private:
    struct addrinfo *m_Addr;
    int m_SockFd;
    bool stopping;

    const std::string server_url = "irc.chat.twitch.tv";
    const std::string server_port = "6667";
    const std::string server_ssl_port = "6697";
    std::string token;

    static addrinfo *get_addrinfo();
    int get_socket();
    void connect();
    void login();
    static void delete_token();
    static std::string message_display(const std::string &prefix, const std::string &trailing);
};


#endif //C_IRC_CLIENT_TWITCH_H
