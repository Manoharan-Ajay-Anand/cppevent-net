#include "socket.hpp"

#include "util.hpp"

#include <cppevent_base/event_loop.hpp>
#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

#include <unistd.h>

#include <algorithm>
#include <stdexcept>

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

cppevent::socket_readable_awaiter cppevent::socket::get_readable() {
    return { m_fd, m_read_status, *m_listener, m_in_buffer, {} };
}

void cppevent::socket::advance_read(long size) {
    m_in_buffer.increment_read_p(size);
}

cppevent::awaitable_task<long> cppevent::socket::read(void* dest, long size, bool read_fully) {
    std::byte* dest_p = static_cast<std::byte*>(dest);
    long total = 0;
    while (size > 0) {
        io_chunk chunk = co_await get_readable();
        if (chunk.m_size == 0) {
            break;
        }
        long transferred = m_in_buffer.read(dest_p, size);
        dest_p += transferred;
        size -= transferred;
        total += transferred;
    }
    if (read_fully && size > 0) {
        throw std::runtime_error("socket read failed: socket closed");
    }
    co_return total;
}

cppevent::awaitable_task<std::string> cppevent::socket::read_line(bool read_fully) {
    std::string result;
    char last_char = '\0';
    bool line_ended = false;
    while (!line_ended) {
        io_chunk chunk = co_await get_readable();
        if (chunk.m_size == 0) {
            break;
        }
        long i = 0;
        for (; i < chunk.m_size; ++i) {
            char c = *(reinterpret_cast<char*>(chunk.m_ptr + i));
            bool newline = c == '\n';
            if (newline || last_char == '\r') {
                i += newline;
                line_ended = true;
                break;
            }
            if (c != '\r') {
                result.push_back(c);
            }
            last_char = c;
        }
        advance_read(i);
    }
    if (read_fully && !line_ended) {
        throw std::runtime_error("socket read_line failed: socket closed");
    }
    co_return std::move(result);
}

cppevent::awaitable_task<long> cppevent::socket::skip(long size, bool skip_fully) {
    long total = 0;
    while (size > 0) {
        io_chunk chunk = co_await get_readable();
        if (chunk.m_size == 0) {
            break;
        }
        long to_skip = std::min(size, chunk.m_size);
        size -= to_skip;
        total += to_skip;
        advance_read(to_skip);
    }
    if (skip_fully && size > 0) {
        throw std::runtime_error("socket skip failed: socket closed");
    }
    co_return total;
}

cppevent::socket_writable_awaiter cppevent::socket::get_writable() {
    return { m_fd, m_write_status, *m_listener, m_out_buffer, {} };
}

void cppevent::socket::advance_write(long size) {
    m_out_buffer.increment_write_p(size);
}

cppevent::awaitable_task<void> cppevent::socket::write(const void* src, long size) {
    const std::byte* src_p = static_cast<const std::byte*>(src);
    while (size > 0) {
        co_await get_writable();
        long transferred = m_out_buffer.write(src_p, size);
        src_p += transferred;
        size -= transferred;
    }
}

cppevent::socket_flush_awaiter cppevent::socket::flush() {
    return { 
        m_fd, m_write_status, *m_listener, m_out_buffer
    };
}
