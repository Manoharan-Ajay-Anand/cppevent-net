#ifndef CPPEVENT_NET_CLIENT_SOCKET_HPP
#define CPPEVENT_NET_CLIENT_SOCKET_HPP

#include <memory>
#include <string>

#include <cppevent_base/task.hpp>

struct addrinfo;

namespace cppevent {

class event_loop;

class socket;

class client_socket {
private:
    ::addrinfo* m_res;
    event_loop& m_loop;
public:
    client_socket(const char* name, const char* service, event_loop& loop);
    client_socket(const std::string& name, const std::string& service, event_loop& loop);
    ~client_socket();

    awaitable_task<std::unique_ptr<socket>> connect();
};

}

#endif
