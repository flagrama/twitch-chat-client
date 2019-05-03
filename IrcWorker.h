//
// Created by vcunningham on 5/2/19.
//

#ifndef TWITCH_CHAT_IRCWORKER_H
#define TWITCH_CHAT_IRCWORKER_H

#include <thread>
#include <mutex>
#include <glibmm/ustring.h>
#include "Twitch.h"

class ThreadWindow;


class IrcWorker {
public:
    IrcWorker();

    void process_responses(ThreadWindow* caller);
    void get_data(Glib::ustring* message);
    void stop_processing_responses();
    bool has_stopped() const;

private:
    mutable std::mutex m_Mutex;
    Twitch m_Worker;
    std::thread* m_WorkerThread;

    std::queue<std::string> m_response_queue;
    bool m_shall_stop;
    bool m_has_stopped;
    Glib::ustring m_message;
};


#endif //TWITCH_CHAT_IRCWORKER_H
