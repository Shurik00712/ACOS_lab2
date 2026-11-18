#pragma once

enum class ClientState {
    Start, Connecting, Connected, Error,
    Sending, Waiting, Closed, Processing
};

enum class ServerState {
    Start, Listening, Listened, Error,
    Waiting, Processing, Sending
};

inline const char* to_string(ClientState s) {
    switch (s) {
        case ClientState::Start:      return "Start";
        case ClientState::Connecting: return "Connecting";
        case ClientState::Connected:  return "Connected";
        case ClientState::Sending:    return "Sending";
        case ClientState::Waiting:    return "Waiting";
        case ClientState::Processing: return "Processing";
        case ClientState::Error:      return "Error";
        case ClientState::Closed:     return "Closed";
    }
    return "?";
}

inline const char* to_string(ServerState s) {
    switch (s) {
        case ServerState::Start:      return "Start";
        case ServerState::Listening:  return "Listening";
        case ServerState::Listened:   return "Listened";
        case ServerState::Waiting:    return "Waiting";
        case ServerState::Sending:    return "Sending";
        case ServerState::Processing: return "Processing";
        case ServerState::Error:      return "Error";
    }
    return "?";
}

inline bool is_valid_transition(ClientState from, ClientState to) {
    if (to == ClientState::Error)
        return from != ClientState::Closed && from != ClientState::Start;

    switch (from) {
        case ClientState::Start:      return to == ClientState::Connecting;
        case ClientState::Connecting: return to == ClientState::Connected;
        case ClientState::Connected:  return to == ClientState::Sending;
        case ClientState::Sending:    return to == ClientState::Waiting;
        case ClientState::Waiting:    return to == ClientState::Processing
                                          || to == ClientState::Closed;
        case ClientState::Processing: return to == ClientState::Sending;
        case ClientState::Error:
        case ClientState::Closed:     return false;
    }
    return false;
}

inline bool is_valid_transition(ServerState from, ServerState to) {
    if (to == ServerState::Error)
        return from != ServerState::Start;

    switch (from) {
        case ServerState::Start:      return to == ServerState::Listening;
        case ServerState::Listening:  return to == ServerState::Listened;
        case ServerState::Listened:   return to == ServerState::Waiting;
        case ServerState::Waiting:    return to == ServerState::Processing
                                          || to == ServerState::Listening;
        case ServerState::Processing: return to == ServerState::Sending;
        case ServerState::Sending:    return to == ServerState::Waiting
                                          || to == ServerState::Listening;
        case ServerState::Error:      return to == ServerState::Listening;
    }
    return false;
}