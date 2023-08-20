#include "socket_read_awaiters.hpp"

#include "util.hpp"

#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

bool cppevent::socket_readable_awaiter::read_available() {
    if (m_status == OP_STATUS::SUCCESS && m_buffer.available() == 0) {
        m_status = read_file(m_fd, m_buffer);
    }
    m_chunk = m_buffer.get_read_chunk();
    return !(m_status == OP_STATUS::BLOCK && m_chunk.m_size == 0);
}

bool cppevent::socket_readable_awaiter::await_ready() {
    return read_available();
}

void cppevent::socket_readable_awaiter::set_read_handler(std::coroutine_handle<> handle) {
    m_listener.set_read_handler([this, handle]() {
        m_status = read_file(m_fd, m_buffer);
        if (read_available()) {
            return handle.resume();
        }
        set_read_handler(handle);
    });
}

void cppevent::socket_readable_awaiter::await_suspend(std::coroutine_handle<> handle) {
    set_read_handler(handle);
}

cppevent::io_chunk cppevent::socket_readable_awaiter::await_resume() {
    if (m_status == OP_STATUS::ERROR) {
        throw_error("socket read failed: ");
    }
    return m_chunk;
}
