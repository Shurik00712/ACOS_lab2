#include "socket_utils.hpp"
#include "states.hpp"
#include "protocol.hpp"
#include <cstdio>

using namespace std;


struct Client {
    public:
        Client(string ip_addr, uint16_t port_num)
            : ip(move(ip_addr)), port(port_num), addr(socket_utils::make_address(ip, port)) {}
        ~Client() { socket_utils::close_socket(socket_fd); }
        Client(const Client&)            = delete;
        Client& operator=(const Client&) = delete;
        void connect_to_server() {
            if (socket_fd != -1)
                throw std::system_error(errno, std::generic_category(), "socket created already");
            socket_fd = socket_utils::create_tcp_socket();
            state = ClientState::Connecting;
            
            try {
                socket_utils::connect_socket(socket_fd, addr);
            } catch (...) {
                state = ClientState::Error;
                throw;
            }

            state = ClientState::Connected;
            std::fprintf(stderr, "[CLIENT] Connected to %s:%u\n",
                        ip.c_str(), static_cast<unsigned>(port));
        }

        void send_msg(const std::string& msg) {
            if (socket_fd == -1 || (state != ClientState::Connected && state != ClientState::Processing)) {
                throw std::logic_error("Client is not ready to sent a message");
            }
            state = ClientState::Sending;
            try {
                socket_utils::send_all(socket_fd, msg);
            } catch (const std::exception&) {
                state = ClientState::Error;
                throw;
            }
            state = ClientState::Waiting;
        }

        std::string recv_msg() {
            if (state != ClientState::Waiting) {
                throw std::logic_error("Client is not waiting for a response");
            }

            std::string message;
            try {
                if (socket_utils::receive_line(socket_fd, message, protocol::kMaxMessageSize)) {
                    state = ClientState::Processing;
                    return message;
                }
            } catch (const std::exception&) {
                state = ClientState::Error;
                throw;
            }
            state = ClientState::Closed;
            throw std::runtime_error("Server closed the connection before sending a response");
        }

    private:
        int socket_fd = -1;
        std::string ip;
        uint16_t port = 0;
        sockaddr_in addr{};
        ClientState state = ClientState::Start;

};

static uint16_t parse_port(const char* text) {
    std::size_t pos = 0;
    const unsigned long value = std::stoul(text, &pos);
    if (text[pos] != '\0' || value == 0 || value > 65535) {
        throw std::invalid_argument("Port must be an integer from 1 to 65535");
    }
    return static_cast<uint16_t>(value);
}

int main(int argc, char **argv) {
    const std::string ip = (argc > 1) ? argv[1] : "127.0.0.1";
    const uint16_t port = (argc > 2) ? parse_port(argv[2]) : 8080;

    try {
        Client c(ip, port);
        c.connect_to_server();

        c.send_msg(protocol::kPing + "\n");
        std::string resp = c.recv_msg();
        std::printf("[CLIENT] Received: %s\n", resp.c_str());
        if (resp != protocol::kPong) {
            throw std::runtime_error("Unexpected server response: " + resp);
        }
    } catch (const std::exception &e) {
        std::fprintf(stderr, "[CLIENT] failed: %s\n", e.what());
        return 1;
    }
    return 0;
}