#ifndef CPPEVENT_NET_SOCKET_WRITE_AWAITERS_HPP
#define CPPEVENT_NET_SOCKET_WRITE_AWAITERS_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <coroutine>
#include <cstddef>

namespace cppevent {

class event_listener;

struct socket_write_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    
    const std::byte* m_src;
    long m_size;

    void attempt_write();
    void write_loop();
    void on_write_available(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    void await_resume();
};

struct socket_flush_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    
    void on_write_available(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    void await_resume();
};

}

#endif
