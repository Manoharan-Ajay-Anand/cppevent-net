#ifndef CPPEVENT_NET_SOCKET_READ_AWAITERS_HPP
#define CPPEVENT_NET_SOCKET_READ_AWAITERS_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <coroutine>
#include <cstddef>
#include <string>

namespace cppevent {

class event_listener;

struct socket_readable_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    io_chunk m_chunk;

    bool read_available();
    void set_read_handler(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    io_chunk await_resume();
};

}

#endif
