#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <stdexcept>
#include <system_error>

namespace socket_utils {
    int create_tcp_socket();
    sockaddr_in make_address(const std::string& ip, uint16_t port);
    void set_reuse_address(int socket_fd);
    void bind_socket(int socket_fd, const sockaddr_in& address);
    void listen_socket(int socket_fd, int backlog);
    int accept_client(int server_fd);
    void connect_socket(int socket_fd, const sockaddr_in& address);
    void send_all(int socket_fd, const std::string& data);
    bool receive_line(int socket_fd, std::string& line, std::size_t max_size);
    void close_socket(int& socket_fd) noexcept;
}