add_library(cppevent-net STATIC
    src/cppevent_net/socket.cpp
    src/cppevent_net/server_socket.cpp
    src/cppevent_net/util.cpp
)

target_include_directories(cppevent-net PUBLIC src)

target_link_libraries(cppevent-net PUBLIC cppevent-base)
