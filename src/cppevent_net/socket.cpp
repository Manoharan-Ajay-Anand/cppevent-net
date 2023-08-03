#include "socket.hpp"

#include "util.hpp"

#include <cppevent_base/event_loop.hpp>
#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

#include <unistd.h>

#include <algorithm>

cppevent::socket::socket(int socket_fd, event_loop& loop): m_fd(socket_fd) {
    m_listener = loop.get_io_listener(m_fd);
    m_read_status = SOCKET_OP_STATUS::SOCKET_OP_SUCCESS;
    m_write_status = SOCKET_OP_STATUS::SOCKET_OP_SUCCESS;
}

cppevent::socket::~socket() {
    int status = close(m_fd);
    cppevent::throw_if_error(status, "Failed to close socket fd: ");
    m_listener->detach();
}

void cppevent::socket::read_helper(std::byte*& dest, long& total, long& size) {
    long transferred = m_in_buffer.read(dest, size);
    dest += transferred;
    total += transferred;
    size -= transferred;
}

void cppevent::socket::read_str_helper(std::string& result, long& size) {
    io_chunk chunk;
    while (can_read_buffer(chunk, m_in_buffer)) {
        long size_to_read = std::min(size, chunk.m_size);
        result.append(reinterpret_cast<char*>(chunk.m_ptr), size_to_read);
        size -= size_to_read;
        m_in_buffer.increment_read_p(size_to_read);
    }
}

void cppevent::socket::read_line_helper(std::string& result, char& prev, bool& line_ended) {
    io_chunk chunk;
    while (can_read_buffer(chunk, m_in_buffer)) {
        for (long i = 0; i < chunk.m_size; ++i) {
            char c = *(reinterpret_cast<char*>(chunk.m_ptr + i));
            bool is_line_feed = c == '\n';
            if (is_line_feed || prev == '\r') {
                line_ended = true;
                return m_in_buffer.increment_read_p(i + is_line_feed);
            }
            prev = c;
            if (c != '\r') {
                result += c;
            }
        }
        m_in_buffer.increment_read_p(chunk.m_size);
    }
}

void cppevent::socket::write_helper(const std::byte*& src, long& size) {
    long transferred = m_out_buffer.write(src, size);
    src += transferred;
    size -= transferred;
}

cppevent::awaitable_task<long> cppevent::socket::read(void* dest, long size, bool read_fully) {
    std::byte* dest_ptr = static_cast<std::byte*>(dest);
    long total_size_read = 0;

    read_helper(dest_ptr, total_size_read, size);

    while (size > 0 && m_read_status != SOCKET_OP_STATUS::SOCKET_OP_CLOSE) {
        switch (m_read_status) {
            case SOCKET_OP_STATUS::SOCKET_OP_ERROR:
                throw_errno("socket read failed: ");
            case SOCKET_OP_STATUS::SOCKET_OP_BLOCK:
                co_await read_awaiter { *m_listener };
            default:
                m_read_status = recv_to_buffer(m_fd, m_in_buffer);
                read_helper(dest_ptr, total_size_read, size);
        }
    }

    if (m_read_status == SOCKET_OP_STATUS::SOCKET_OP_CLOSE && size > 0 && read_fully) {
        throw std::runtime_error("socket read failed: socket closed");
    }
    co_return total_size_read;
}

cppevent::awaitable_task<std::string> cppevent::socket::read_str(long size, bool read_fully) {
    std::string result;
    result.reserve(size);

    read_str_helper(result, size);

    while (size > 0 && m_read_status != SOCKET_OP_STATUS::SOCKET_OP_CLOSE) {
        switch (m_read_status) {
            case SOCKET_OP_STATUS::SOCKET_OP_ERROR:
                throw_errno("socket read_str failed: ");
            case SOCKET_OP_STATUS::SOCKET_OP_BLOCK:
                co_await read_awaiter { *m_listener };
            default:
                m_read_status = recv_to_buffer(m_fd, m_in_buffer);
                read_str_helper(result, size);
        }
    }

    if (m_read_status == SOCKET_OP_STATUS::SOCKET_OP_CLOSE && size > 0 && read_fully) {
        throw std::runtime_error("socket read_str failed: socket closed");
    }
    co_return std::move(result);
}

cppevent::awaitable_task<std::string> cppevent::socket::read_line(bool read_fully) {
    std::string result;
    char prev = 0;
    bool line_ended = false;

    read_line_helper(result, prev, line_ended);

    while (!line_ended && m_read_status != SOCKET_OP_STATUS::SOCKET_OP_CLOSE) {
        switch (m_read_status) {
            case SOCKET_OP_STATUS::SOCKET_OP_ERROR:
                throw_errno("socket read_str failed: ");
            case SOCKET_OP_STATUS::SOCKET_OP_BLOCK:
                co_await read_awaiter { *m_listener };
            default:
                m_read_status = recv_to_buffer(m_fd, m_in_buffer);
                read_line_helper(result, prev, line_ended);
        }
    }

    if (m_read_status == SOCKET_OP_STATUS::SOCKET_OP_CLOSE && !line_ended && read_fully) {
        throw std::runtime_error("socket read_line failed: socket closed");
    }
    co_return std::move(result);
}

cppevent::awaitable_task<void> cppevent::socket::write(const void* src, long size) {
    const std::byte* src_ptr = static_cast<const std::byte*>(src);
    long total_size_written = 0;

    write_helper(src_ptr, size);

    while (size > 0) {
        switch (m_write_status) {
            case SOCKET_OP_STATUS::SOCKET_OP_ERROR:
                throw_errno("socket write failed: ");
            case SOCKET_OP_STATUS::SOCKET_OP_BLOCK:
                co_await write_awaiter { *m_listener };
            default:
                m_write_status = send_from_buffer(m_fd, m_out_buffer);
                write_helper(src_ptr, size);
        }
    }
}

cppevent::awaitable_task<void> cppevent::socket::flush() {
    while (m_out_buffer.available() > 0) {
        switch (m_write_status) {
            case SOCKET_OP_STATUS::SOCKET_OP_ERROR:
                throw_errno("socket flush failed: ");
            case SOCKET_OP_STATUS::SOCKET_OP_BLOCK:
                co_await write_awaiter { *m_listener };
            default:
                m_write_status = send_from_buffer(m_fd, m_out_buffer);
        }
    }
}
