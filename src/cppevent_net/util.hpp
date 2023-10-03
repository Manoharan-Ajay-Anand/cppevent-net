#ifndef CPPEVENT_NET_UTIL_HPP
#define CPPEVENT_NET_UTIL_HPP

#include "types.hpp"

#include <cppevent_base/byte_buffer.hpp>

#include <unistd.h>
#include <sys/uio.h>

#include <cerrno>

#include <array>

struct addrinfo;

namespace cppevent {

template<long BUFFER_SIZE>
inline bool can_read_buffer(io_chunk& chunk, byte_buffer<BUFFER_SIZE>& buffer) {
    chunk = buffer.get_read_chunk();
    return chunk.m_size > 0;
}

template<long BUFFER_SIZE>
inline bool can_read_buffer(io_chunk_group& group, byte_buffer<BUFFER_SIZE>& buffer) {
    group = buffer.get_read_chunks();
    return group.m_count > 0;
}

template<long BUFFER_SIZE>
inline bool can_write_buffer(io_chunk& chunk, byte_buffer<BUFFER_SIZE>& buffer) {
    chunk = buffer.get_write_chunk();
    return chunk.m_size > 0;
}

template<long BUFFER_SIZE>
inline bool can_write_buffer(io_chunk_group& group, byte_buffer<BUFFER_SIZE>& buffer) {
    group = buffer.get_write_chunks();
    return group.m_count > 0;
}

template<long BUFFER_SIZE>
inline OP_STATUS read_file(int fd, byte_buffer<BUFFER_SIZE>& buffer) {
    io_chunk_group group;
    while (can_write_buffer(group, buffer)) {
        std::array<iovec, 2> arr = {
            iovec { group.m_chunks[0].m_ptr, static_cast<size_t>(group.m_chunks[0].m_size) },
            iovec { group.m_chunks[1].m_ptr, static_cast<size_t>(group.m_chunks[1].m_size) }
        };
        auto size_read = ::readv(fd, arr.data(), group.m_count);
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
    io_chunk_group group;
    while (can_read_buffer(group, buffer)) {
        std::array<iovec, 2> arr = {
            iovec { group.m_chunks[0].m_ptr, static_cast<size_t>(group.m_chunks[0].m_size) },
            iovec { group.m_chunks[1].m_ptr, static_cast<size_t>(group.m_chunks[1].m_size) }
        };
        auto size_written = ::writev(fd, arr.data(), group.m_count);
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

void get_addrinfo(const char* name, const char* service, addrinfo** res);

}

#endif
