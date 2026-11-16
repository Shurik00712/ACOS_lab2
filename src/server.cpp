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
            fprintf(stderr, "[SERVER] Client connected\n");
            try {
                string st;
                for (;;) {
                    state = ServerState::Waiting;
                    bool received = socket_utils::receive_line(client_fd, st, protocol::kMaxMessageSize);
                    if (!received) {
                        fprintf(stderr, "[SERVER] Client disconnected\n");
                        break;
                    }
                    fprintf(stderr, "[SERVER] Received: %s\n", st.c_str());
                    state = ServerState::Processing; 
                    if (st == protocol::kPing) {
                        state = ServerState::Sending;
                        socket_utils::send_all(
                            client_fd, string(protocol::kPong) + "\n");
                        fprintf(stderr, "[SERVER] Sent: PONG\n");
                    } else if (st == protocol::kHistory) {
                        state = ServerState::Sending;
                        socket_utils::send_all(
                            client_fd,
                            protocol::kHistoryPrefix + std::to_string(history.size()) + "\n");
                        for (const auto& entry : history)
                            socket_utils::send_all(client_fd, entry + "\n");
                    } else if (st == protocol::kQuit) {
                        state = ServerState::Sending;
                        socket_utils::send_all(client_fd, protocol::kBye + "\n");
                        break;
                    } else {
                        history.push_back(st);         
                        state = ServerState::Sending;
                        socket_utils::send_all(client_fd, protocol::kOk + "\n");
                    }
                }
            } catch (const exception& error) {
                state = ServerState::Error;
                fprintf(stderr, "[SERVER] Client session error: %s\n", error.what());
            }
            socket_utils::close_socket(client_fd);
            state = ServerState::Listening;
        }
        void start() {
            create_socket();
            bind_and_listen();

            fprintf(stderr, "[SERVER] Server started on %s:%u\n",
                        ip.c_str(), static_cast<unsigned>(port));
            state = ServerState::Listening;
            for (;;) {
                accept_client();
            }
        }
    private:
        void create_socket() {
            if (socket_fd != -1) {
                throw logic_error("Server socket is already created");
            }

            socket_fd = socket_utils::create_tcp_socket();
            socket_utils::set_reuse_address(socket_fd);
        }
        void bind_and_listen() {
            socket_utils::bind_socket(socket_fd, addr);
            socket_utils::listen_socket(socket_fd, 128);
            fprintf(stderr, "[SERVER] Waiting for connection...\n");
        }
        int socket_fd = -1;
        string ip;
        uint16_t port = 0;
        sockaddr_in addr{};
        ServerState state = ServerState::Start;
        vector<string> history;

};

static uint16_t parse_port(const char* text) {
    size_t pos = 0;
    const unsigned long value = stoul(text, &pos);
    if (text[pos] != '\0' || value == 0 || value > 65535) {
        throw invalid_argument("Port must be an integer from 1 to 65535");
    }
    return static_cast<uint16_t>(value);
}

int main(int argc, char** argv) {
    try {
        const string ip = (argc > 1) ? argv[1] : "0.0.0.0";
        const uint16_t port = (argc > 2) ? parse_port(argv[2]) : 8080;
        Server s(ip, port);
        s.start();           
    } catch (const exception &e) {
        fprintf(stderr, "[SERVER] failed: %s\n", e.what());
        return 1;
    }
    return 0;
}