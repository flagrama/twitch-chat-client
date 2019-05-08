//
// Created by vcunningham on 5/2/19.
//

#include "IrcWorker.h"
#include "ThreadWindow.h"
#include <sstream>
#include <queue>

IrcWorker::IrcWorker(Twitch* twitch) :
    m_Mutex(),
    m_shall_stop(false),
    m_has_stopped(false),
    m_message(),
    m_response_queue(),
    m_Twitch(twitch),
    m_TwitchThread(nullptr) {}

void IrcWorker::get_data(Glib::ustring *message) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    *message = m_message;
    m_message = "";
}

void IrcWorker::stop_processing_responses() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_shall_stop = true;
    m_Twitch->disconnect();
}

bool IrcWorker::has_stopped() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_has_stopped;
}

void IrcWorker::process_responses(ThreadWindow *caller) {
    m_TwitchThread = new std::thread([this] {
        m_Twitch->read_responses(m_response_queue);
    });

    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_has_stopped = false;
            m_message = "";
        }

        if(!m_shall_stop) {
            while(!m_response_queue.empty()) {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_message += m_response_queue.front();
                m_response_queue.pop();
                caller->notify();
            }

            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_shall_stop = false;
            m_has_stopped = true;
        }
    } while (!m_has_stopped);

    caller->notify();
}
