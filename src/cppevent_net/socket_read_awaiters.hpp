#ifndef CPPEVENT_NET_SOCKET_READ_AWAITERS_HPP
#define CPPEVENT_NET_SOCKET_READ_AWAITERS_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <coroutine>
#include <cstddef>
#include <string>

namespace cppevent {

class event_listener;

struct socket_read_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;
    
    std::byte* m_dest;
    long m_size;
    long m_total_size;
    const bool read_fully;

    void attempt_read();
    void read_loop();
    void on_read_available(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    long await_resume();
};

struct socket_read_line_awaiter {
    const int m_fd;
    OP_STATUS& m_status;
    event_listener& m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE>& m_buffer;

    bool m_line_ended;
    char m_prev;
    
    std::string m_result;
    const bool read_fully;

    void attempt_read();
    void read_loop();
    void on_read_available(std::coroutine_handle<> handle);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    std::string&& await_resume();
};

}

#endif
