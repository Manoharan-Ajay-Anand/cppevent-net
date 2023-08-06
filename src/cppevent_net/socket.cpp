#include "socket.hpp"

#include "util.hpp"

#include <cppevent_base/event_loop.hpp>
#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

#include <unistd.h>

#include <algorithm>

cppevent::socket::socket(int socket_fd, event_loop& loop): m_fd(socket_fd) {
    m_listener = loop.get_io_listener(m_fd);
    m_read_status = OP_STATUS::SUCCESS;
    m_write_status = OP_STATUS::SUCCESS;
}

cppevent::socket::socket(int socket_fd, event_listener* listener): m_fd(socket_fd),
                                                                   m_listener(listener) {
    m_read_status = OP_STATUS::SUCCESS;
    m_write_status = OP_STATUS::SUCCESS;
}

cppevent::socket::~socket() {
    int status = close(m_fd);
    throw_if_error(status, "Failed to close socket fd: ");
    m_listener->detach();
}

cppevent::socket_read_awaiter cppevent::socket::read(void* dest, long size, bool read_fully) {
    return { 
        m_fd, m_read_status, *m_listener, m_in_buffer,
        static_cast<std::byte*>(dest), size, 0, read_fully
    };
}

cppevent::socket_read_line_awaiter cppevent::socket::read_line(bool read_fully) {
    return { 
        m_fd, m_read_status, *m_listener, m_in_buffer,
        false, '\0', {}, read_fully
    };
}

cppevent::socket_write_awaiter cppevent::socket::write(const void* src, long size) {
    return { 
        m_fd, m_read_status, *m_listener, m_out_buffer,
        static_cast<const std::byte*>(src), size
    };
}

cppevent::socket_flush_awaiter cppevent::socket::flush() {
    return { 
        m_fd, m_read_status, *m_listener, m_out_buffer
    };
}
