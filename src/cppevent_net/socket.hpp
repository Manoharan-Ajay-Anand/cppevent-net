#ifndef CPPEVENT_NET_SOCKET_HPP
#define CPPEVENT_NET_SOCKET_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>
#include <cppevent_base/task.hpp>

#include <string>

namespace cppevent {

constexpr long SOCKET_BUFFER_SIZE = 8000;

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

    void read_helper(std::byte*& dest, long& total, long& size);
    void read_line_helper(std::string& result, char& prev, bool& line_ended);

    void write_helper(const std::byte*& src, long& size);
public:
    socket(int socket_fd, event_loop& loop);
    socket(int socket_fd, event_listener* m_listener);
    ~socket();

    awaitable_task<long> read(void* dest, long size, bool read_fully);
    awaitable_task<std::string> read_line(bool read_fully);

    awaitable_task<void> write(const void* src, long size);
    awaitable_task<void> flush();
};

}

#endif
