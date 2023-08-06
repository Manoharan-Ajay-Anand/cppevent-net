#include "socket_write_awaiters.hpp"

#include "util.hpp"

#include <cppevent_base/event_listener.hpp>
#include <cppevent_base/util.hpp>

void cppevent::socket_write_awaiter::attempt_write() {
    long transferred = m_buffer.write(m_src, m_size);
    m_size -= transferred;
    m_src += transferred;
}

void cppevent::socket_write_awaiter::write_loop() {
    while (m_size > 0 && m_status == OP_STATUS::SUCCESS) {
        m_status = write_file(m_fd, m_buffer);
        attempt_write();
    }
}

void cppevent::socket_write_awaiter::on_write_available(std::coroutine_handle<> handle) {
    m_status = OP_STATUS::SUCCESS;
    write_loop();
    if (m_size > 0 && m_status == OP_STATUS::BLOCK) {
        return m_listener.set_write_handler([this, handle]() {
            on_write_available(handle);
        });
    }
    handle.resume();
}

bool cppevent::socket_write_awaiter::await_ready() {
    attempt_write();
    write_loop();
    return !(m_size > 0 && m_status == OP_STATUS::BLOCK);
}

void cppevent::socket_write_awaiter::await_suspend(std::coroutine_handle<> handle) {
    m_listener.set_write_handler([this, handle]() {
        on_write_available(handle);
    });
}

void cppevent::socket_write_awaiter::await_resume() {
    if (m_size == 0) {
        return;
    }
    throw_error("socket write failed: ");
}

void cppevent::socket_flush_awaiter::on_write_available(std::coroutine_handle<> handle) {
    m_status = write_file(m_fd, m_buffer);
    if (m_buffer.available() > 0 && m_status == OP_STATUS::BLOCK) {
        return m_listener.set_write_handler([this, handle]() {
            on_write_available(handle);
        });
    }
    handle.resume();
}

bool cppevent::socket_flush_awaiter::await_ready() {
    m_status = write_file(m_fd, m_buffer);
    return !(m_buffer.available() > 0 && m_status == OP_STATUS::BLOCK);
}

void cppevent::socket_flush_awaiter::await_suspend(std::coroutine_handle<> handle) {
    m_listener.set_write_handler([this, handle]() {
        on_write_available(handle);
    });
}

void cppevent::socket_flush_awaiter::await_resume() {
    if (m_status == OP_STATUS::ERROR) {
        throw_error("socket flush failed: ");
    }
}
