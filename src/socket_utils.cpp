#include "socket_utils.hpp"

using namespace std;

namespace socket_utils {

int create_tcp_socket() {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw system_error(errno, generic_category(), "socket()");
    }
    return fd;
}

sockaddr_in make_address(const string& ip, uint16_t port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    const int result = ::inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    if (result == 0) {
        throw invalid_argument("Invalid IPv4 address: " + ip);
    }
    if (result == -1) {
        throw system_error(errno, generic_category(), "inet_pton()");
    }
    return address;
}

void set_reuse_address(int socket_fd) {
    const int enabled = 1;
    if (::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                     &enabled, sizeof(enabled)) == -1) {
        throw system_error(errno, generic_category(),
                                "setsockopt(SO_REUSEADDR)");
    }
}

void bind_socket(int socket_fd, const sockaddr_in& address) {
    if (::bind(socket_fd, reinterpret_cast<const sockaddr*>(&address),
               sizeof(address)) == -1) {
        throw system_error(errno, generic_category(), "bind()");
    }
}

void listen_socket(int socket_fd, int backlog) {
    if (::listen(socket_fd, backlog) == -1) {
        throw system_error(errno, generic_category(), "listen()");
    }
}

int accept_client(int server_fd) {
    for (;;) {
        const int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            return client_fd;
        }
        if (errno == EINTR) {
            continue;
        }
        throw system_error(errno, generic_category(), "accept()");
    }
}

void connect_socket(int socket_fd, const sockaddr_in& address) {
    if (::connect(socket_fd, reinterpret_cast<const sockaddr*>(&address),
                  sizeof(address)) == -1) {
        throw system_error(errno, generic_category(), "connect()");
    }
}

void send_all(int socket_fd, const string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t count = ::send(socket_fd, data.data() + sent,
                                     data.size() - sent, MSG_NOSIGNAL);
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw system_error(errno, generic_category(), "send()");
        }
        if (count == 0) {
            throw runtime_error("send() returned 0 before all bytes were sent");
        }
        sent += static_cast<size_t>(count);
    }
}

bool receive_line(int socket_fd, string& line, size_t max_size) {
    line.clear();

    for (;;) {
        char ch = '\0';
        const ssize_t count = ::recv(socket_fd, &ch, 1, 0);

        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            throw system_error(errno, generic_category(), "recv()");
        }

        if (count == 0) {
            if (line.empty()) {
                return false; 
            }
            throw runtime_error("Peer closed connection in the middle of a message");
        }

        if (ch == '\n') {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return true;
        }

        if (line.size() >= max_size) {
            throw length_error("Received message is too long");
        }
        line.push_back(ch);
    }
}

void close_socket(int& socket_fd) noexcept {
    if (socket_fd != -1) {
        ::close(socket_fd);
        socket_fd = -1;
    }
}

}
