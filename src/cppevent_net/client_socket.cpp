#include "client_socket.hpp"

#include "socket.hpp"
#include "util.hpp"

#include <cppevent_base/util.hpp>
#include <cppevent_base/event_loop.hpp>
#include <cppevent_base/event_listener.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cerrno>
#include <stdexcept>

cppevent::client_socket::client_socket(const std::string& name,
                                       const std::string& service,
                                       event_loop& loop): m_loop(loop) {
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = ::getaddrinfo(name.c_str(), service.c_str(), &hints, &m_res);
    if (status != 0) {
        throw std::runtime_error(std::string("getaddrinfo failed: ").append(::gai_strerror(status)));
    }                                   
}

cppevent::client_socket::~client_socket() {
    ::freeaddrinfo(m_res);
}

cppevent::awaitable_task<std::unique_ptr<cppevent::socket>> cppevent::client_socket::connect() {
    int fd = ::socket(m_res->ai_family, m_res->ai_socktype, m_res->ai_protocol);
    throw_if_error(fd, "client_socket failed to create socket: ");

    set_non_blocking(fd);

    event_listener* listener = m_loop.get_io_listener(fd);

    int status = ::connect(fd, m_res->ai_addr, m_res->ai_addrlen);
    std::string prefix("client_socket failed to connect: ");

    if (status < 0) {
        if (errno != EAGAIN && errno != EINPROGRESS) {
            throw_errno(prefix);
        }
        co_await write_awaiter { *listener };

        int val;
        socklen_t len = sizeof(val);
        status = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &val, &len);

        if (val != 0) {
            throw std::runtime_error(prefix.append(strerror(val)));
        }
    }

    co_return std::make_unique<socket>(fd, listener);
}
