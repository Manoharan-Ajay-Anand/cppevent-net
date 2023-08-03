#ifndef CPPEVENT_NET_UTIL_HPP
#define CPPEVENT_NET_UTIL_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <sys/socket.h>
#include <cerrno>

namespace cppevent {

template<long BUFFER_SIZE>
inline SOCKET_OP_STATUS recv_to_buffer(int socket_fd, byte_buffer<BUFFER_SIZE>& buffer) {
    io_chunk chunk;
    while ((chunk = buffer.get_write_chunk()).m_size > 0) {
        auto size_read = recv(socket_fd, chunk.m_ptr, chunk.m_size, 0);
        if (size_read > 0) {
            buffer.increment_write_p(size_read);
        } else if (size_read == 0) {
            return SOCKET_OP_STATUS::SOCKET_OP_CLOSE;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SOCKET_OP_STATUS::SOCKET_OP_BLOCK;
        } else {
            return SOCKET_OP_STATUS::SOCKET_OP_ERROR;
        }
    }
    return SOCKET_OP_STATUS::SOCKET_OP_SUCCESS;
}

template<long BUFFER_SIZE>
inline SOCKET_OP_STATUS send_from_buffer(int socket_fd, byte_buffer<BUFFER_SIZE>& buffer) {
    io_chunk chunk;
    while ((chunk = buffer.get_read_chunk()).m_size > 0) {
        auto size_written = send(socket_fd, chunk.m_ptr, chunk.m_size, 0);
        if (size_written > 0) {
            buffer.increment_read_p(size_written);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SOCKET_OP_STATUS::SOCKET_OP_BLOCK;
        } else {
            return SOCKET_OP_STATUS::SOCKET_OP_ERROR;
        }
    }
    return SOCKET_OP_STATUS::SOCKET_OP_SUCCESS;
}

void set_non_blocking(int fd);

}

#endif
