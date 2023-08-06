#include "util.hpp"

#include <fcntl.h>
#include <netdb.h>

#include <cppevent_base/util.hpp>

#include <stdexcept>

void cppevent::set_non_blocking(int fd) {
    int flags = ::fcntl(fd, F_GETFL);
    int status = ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    throw_if_error(status, "set non_blocking failed: ");
}

void cppevent::get_addrinfo(const char* name, const char* service, addrinfo** res) {
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = ::getaddrinfo(name, service, &hints, res);
    if (status != 0) {
        throw std::runtime_error(
                std::string("getaddrinfo failed: ").append(::gai_strerror(status)));
    }
}
