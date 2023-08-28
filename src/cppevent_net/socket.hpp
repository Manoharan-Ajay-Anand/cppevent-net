#ifndef CPPEVENT_NET_SOCKET_HPP
#define CPPEVENT_NET_SOCKET_HPP

#include "types.hpp"
#include "socket_read_awaiters.hpp"
#include "socket_write_awaiters.hpp"

#include <cppevent_base/byte_buffer.hpp>
#include <cppevent_base/task.hpp>

#include <string>

namespace cppevent {

class event_loop;

class event_listener;

class socket {
private:
    const int m_fd;
    event_listener* m_listener;
    byte_buffer<SOCKET_BUFFER_SIZE> m_in_buffer;
    byte_buffer<SOCKET_BUFFER_SIZE> m_out_buffer;
    OP_STATUS m_read_status;
    OP_STATUS m_write_status;

public:
    socket(int socket_fd, event_loop& loop);
    socket(int socket_fd, event_listener* m_listener);
    ~socket();

    socket_readable_awaiter get_readable();
    void advance_read(long size);

    awaitable_task<long> read(void* dest, long size, bool read_fully);
    awaitable_task<std::string> read_line(bool read_fully);
    awaitable_task<long> skip(long size, bool skip_fully);

    socket_writable_awaiter get_writable();
    void advance_write(long size);

    awaitable_task<void> write(const void* src, long size);
    socket_flush_awaiter flush();
};

}

#endif
