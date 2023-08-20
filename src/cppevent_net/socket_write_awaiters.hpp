#ifndef CPPEVENT_NET_SOCKET_WRITE_AWAITERS_HPP
#define CPPEVENT_NET_SOCKET_WRITE_AWAITERS_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <coroutine>
#include <cstddef>

namespace cppevent {

class event_listener;

struct socket_writable_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    io_chunk m_chunk;

    bool write_available();
    void set_write_handler(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    io_chunk await_resume();
};

struct socket_flush_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    
    bool flushed();
    void set_flush_handler(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    void await_resume();
};

}

#endif
