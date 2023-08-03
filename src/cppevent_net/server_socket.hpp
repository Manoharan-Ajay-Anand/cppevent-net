#ifndef CPPEVENT_NET_SERVER_SOCKET_HPP
#define CPPEVENT_NET_SERVER_SOCKET_HPP

#include <string>
#include <memory>

#include <cppevent_base/task.hpp>

namespace cppevent {

class event_listener;

class event_loop;

class socket;

class server_socket {
private:
    event_loop& m_loop;
    int m_fd;
    event_listener* m_listener;
public:
    server_socket(const std::string& name, const std::string& service, event_loop& loop);
    ~server_socket();

    awaitable_task<std::unique_ptr<socket>> accept();
};

}

#endif
