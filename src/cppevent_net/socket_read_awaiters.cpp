#include "socket_read_awaiters.hpp"

#include "util.hpp"

#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

void cppevent::socket_read_awaiter::attempt_read() {
    long transferred = m_buffer.read(m_dest, m_size);
    m_size -= transferred;
    m_total_size += transferred;
    m_dest += transferred;
}

void cppevent::socket_read_awaiter::read_loop() {
    while (m_size > 0 && m_status == OP_STATUS::SUCCESS) {
        m_status = read_file(m_fd, m_buffer);
        attempt_read();
    }
}

void cppevent::socket_read_awaiter::on_read_available(std::coroutine_handle<> handle) {
    m_status = OP_STATUS::SUCCESS;
    read_loop();
    if (m_size > 0 && m_status == OP_STATUS::BLOCK) {
        return m_listener.set_read_handler([this, handle]() {
            on_read_available(handle);
        });
    }
    handle.resume();
}

bool cppevent::socket_read_awaiter::await_ready() {
    attempt_read();
    read_loop();
    return !(m_size > 0 && m_status == OP_STATUS::BLOCK);
}

void cppevent::socket_read_awaiter::await_suspend(std::coroutine_handle<> handle) {
    m_listener.set_read_handler([this, handle]() {
        on_read_available(handle);
    });
}

long cppevent::socket_read_awaiter::await_resume() {
    if (m_size == 0 || (m_status == OP_STATUS::CLOSE && !read_fully)) {
        return m_total_size;
    } else if (m_status == OP_STATUS::ERROR) {
        throw_error("socket read failed: ");
    }
    throw std::runtime_error("socket read failed: socket closed");
}

void cppevent::socket_read_line_awaiter::attempt_read() {
    io_chunk chunk;
    while (can_read_buffer(chunk, m_buffer)) {
        for (long i = 0; i < chunk.m_size; ++i) {
            char c = *(reinterpret_cast<char*>(chunk.m_ptr + i));
            bool is_line_feed = c == '\n';
            if (is_line_feed || m_prev == '\r') {
                m_line_ended = true;
                return m_buffer.increment_read_p(i + is_line_feed);
            }
            m_prev = c;
            if (c != '\r') {
                m_result += c;
            }
        }
        m_buffer.increment_read_p(chunk.m_size);
    }
}

void cppevent::socket_read_line_awaiter::read_loop() {
    while (!m_line_ended && m_status == OP_STATUS::SUCCESS) {
        m_status = read_file(m_fd, m_buffer);
        attempt_read();
    }
}

void cppevent::socket_read_line_awaiter::on_read_available(std::coroutine_handle<> handle) {
    m_status = OP_STATUS::SUCCESS;
    read_loop();
    if (!m_line_ended && m_status == OP_STATUS::BLOCK) {
        return m_listener.set_read_handler([this, handle]() {
            on_read_available(handle);
        });
    }
    handle.resume();
}

bool cppevent::socket_read_line_awaiter::await_ready() {
    attempt_read();
    read_loop();
    return !(!m_line_ended && m_status == OP_STATUS::BLOCK);
}

void cppevent::socket_read_line_awaiter::await_suspend(std::coroutine_handle<> handle) {
    m_listener.set_read_handler([this, handle]() {
        on_read_available(handle);
    });
}

std::string&& cppevent::socket_read_line_awaiter::await_resume() {
    if (m_line_ended || (m_status == OP_STATUS::CLOSE && !read_fully)) {
        return std::move(m_result);
    } else if (m_status == OP_STATUS::ERROR) {
        throw_error("socket read_line failed: ");
    }
    throw std::runtime_error("socket read_line failed: socket closed");
}
