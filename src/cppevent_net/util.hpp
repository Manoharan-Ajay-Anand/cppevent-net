#ifndef CPPEVENT_NET_UTIL_HPP
#define CPPEVENT_NET_UTIL_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <unistd.h>
#include <cerrno>

namespace cppevent {

template<long BUFFER_SIZE>
inline bool can_read_buffer(io_chunk& chunk, byte_buffer<BUFFER_SIZE>& buffer) {
    chunk = buffer.get_read_chunk();
    return chunk.m_size > 0;
}

template<long BUFFER_SIZE>
inline bool can_write_buffer(io_chunk& chunk, byte_buffer<BUFFER_SIZE>& buffer) {
    chunk = buffer.get_write_chunk();
    return chunk.m_size > 0;
}

template<long BUFFER_SIZE>
inline OP_STATUS read_file(int fd, byte_buffer<BUFFER_SIZE>& buffer) {
    io_chunk chunk;
    while (can_write_buffer(chunk, buffer)) {
        auto size_read = ::read(fd, chunk.m_ptr, chunk.m_size);
        if (size_read > 0) {
            buffer.increment_write_p(size_read);
        } else if (size_read == 0) {
            return OP_STATUS::CLOSE;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return OP_STATUS::BLOCK;
        } else {
            return OP_STATUS::ERROR;
        }
    }
    return OP_STATUS::SUCCESS;
}

template<long BUFFER_SIZE>
inline OP_STATUS write_file(int fd, byte_buffer<BUFFER_SIZE>& buffer) {
    io_chunk chunk;
    while (can_read_buffer(chunk, buffer)) {
        auto size_written = ::write(fd, chunk.m_ptr, chunk.m_size);
        if (size_written > 0) {
            buffer.increment_read_p(size_written);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return OP_STATUS::BLOCK;
        } else {
            return OP_STATUS::ERROR;
        }
    }
    return OP_STATUS::SUCCESS;
}

void set_non_blocking(int fd);

}

#endif
