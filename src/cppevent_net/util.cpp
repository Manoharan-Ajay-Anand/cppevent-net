#include "util.hpp"

#include <fcntl.h>

#include <cppevent_base/util.hpp>

void cppevent::set_non_blocking(int fd) {
    int flags = ::fcntl(fd, F_GETFL);
    int status = ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    throw_if_error(status, "set non_blocking failed: ");
}
