#include "states.hpp"
#include "protocol.hpp"
#include "socket_utils.hpp"
#include <cstdio>
using namespace std;


struct Server {
    public:
        Server(string ip_addr, uint16_t port_num)
            : ip(move(ip_addr)), port(port_num), addr(socket_utils::make_address(ip, port)) {}
        ~Server() { socket_utils::close_socket(socket_fd); }
        Server(const Server&)            = delete;
        Server& operator=(const Server&) = delete;
        void accept_client() {
            int client_fd = socket_utils::accept_client(socket_fd);
            state = ServerState::Listened;
            std::fprintf(stderr, "[SERVER] Client connected\n");
            try {
                std::string st;
                for (;;) {
                    state = ServerState::Waiting;
                    bool received = socket_utils::receive_line(client_fd, st, protocol::kMaxMessageSize);
                    if (!received) {
                        std::fprintf(stderr, "[SERVER] Client disconnected\n");
                        break;
                    }
                    std::fprintf(stderr, "[SERVER] Received: %s\n", st.c_str());
                    state = ServerState::Processing; 
                    if (st == protocol::kPing) {
                        state = ServerState::Sending;
                        socket_utils::send_all(
                            client_fd, std::string(protocol::kPong) + "\n");
                        std::fprintf(stderr, "[SERVER] Sent: PONG\n");
                    } else {
                        socket_utils::send_all(
                            client_fd, std::string(protocol::kError) + "\n");
                        std::fprintf(stderr, "[SERVER] Invalid request; sent ERR\n");
                        break;
                    }
                }
            } catch (const std::exception& error) {
                state = ServerState::Error;
                std::fprintf(stderr, "[SERVER] Client session error: %s\n", error.what());
            }
            socket_utils::close_socket(client_fd);
            state = ServerState::Listening;
        }
        void start() {
            create_socket();
            bind_and_listen();

            std::fprintf(stderr, "[SERVER] Server started on %s:%u\n",
                        ip.c_str(), static_cast<unsigned>(port));
            state = ServerState::Listening;
            for (;;) {
                accept_client();
            }
        }
    private:
        void create_socket() {
            if (socket_fd != -1) {
                throw std::logic_error("Server socket is already created");
            }

            socket_fd = socket_utils::create_tcp_socket();
            socket_utils::set_reuse_address(socket_fd);
        }
        void bind_and_listen() {
            socket_utils::bind_socket(socket_fd, addr);
            socket_utils::listen_socket(socket_fd, 128);
            std::fprintf(stderr, "[SERVER] Waiting for connection...\n");
        }
        int socket_fd = -1;
        std::string ip;
        uint16_t port = 0;
        sockaddr_in addr{};
        ServerState state = ServerState::Start;

};

static uint16_t parse_port(const char* text) {
    std::size_t pos = 0;
    const unsigned long value = std::stoul(text, &pos);
    if (text[pos] != '\0' || value == 0 || value > 65535) {
        throw std::invalid_argument("Port must be an integer from 1 to 65535");
    }
    return static_cast<uint16_t>(value);
}

int main(int argc, char** argv) {
    try {
        const std::string ip = (argc > 1) ? argv[1] : "0.0.0.0";
        const uint16_t port = (argc > 2) ? parse_port(argv[2]) : 8080;
        Server s(ip, port);
        s.start();           
    } catch (const std::exception &e) {
        std::fprintf(stderr, "[SERVER] failed: %s\n", e.what());
        return 1;
    }
    return 0;
}