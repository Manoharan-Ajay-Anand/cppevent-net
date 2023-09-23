#include "socket_write_awaiters.hpp"

#include "util.hpp"

#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

bool cppevent::socket_writable_awaiter::write_available() {
    m_chunk = m_buffer.get_write_chunk();
    return !(m_status == OP_STATUS::BLOCK && m_chunk.m_size == 0);
}

bool cppevent::socket_writable_awaiter::await_ready() {
    if (m_status == OP_STATUS::SUCCESS && m_buffer.capacity() == 0) {
        m_status = write_file(m_fd, m_buffer);
    }
    return write_available();
}

void cppevent::socket_writable_awaiter::set_write_handler(std::coroutine_handle<> handle) {
    m_listener.set_write_handler([this, handle]() {
        m_status = write_file(m_fd, m_buffer);
        if (write_available()) {
            return handle.resume();
        }
        set_write_handler(handle);
    });
}

void cppevent::socket_writable_awaiter::await_suspend(std::coroutine_handle<> handle) {
    set_write_handler(handle);
}

cppevent::io_chunk cppevent::socket_writable_awaiter::await_resume() {
    if (m_status == OP_STATUS::ERROR) {
        throw_error("socket read failed: ");
    }
    return m_chunk;
}

bool cppevent::socket_flush_awaiter::flushed() {
    return !(m_status == OP_STATUS::BLOCK && m_buffer.available() > 0);
}

bool cppevent::socket_flush_awaiter::await_ready() {
    if (m_status == OP_STATUS::SUCCESS && m_buffer.available() > 0) {
        m_status = write_file(m_fd, m_buffer);
    }
    return flushed();
}

void cppevent::socket_flush_awaiter::set_flush_handler(std::coroutine_handle<> handle) {
    m_listener.set_write_handler([this, handle]() {
        m_status = write_file(m_fd, m_buffer);
        if (flushed()) {
            return handle.resume();
        }
        set_flush_handler(handle);
    });
}

void cppevent::socket_flush_awaiter::await_suspend(std::coroutine_handle<> handle) {
    set_flush_handler(handle);
}

void cppevent::socket_flush_awaiter::await_resume() {
    if (m_status == OP_STATUS::ERROR) {
        throw_error("socket flush failed: ");
    }
}
