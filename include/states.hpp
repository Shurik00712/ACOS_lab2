#pragma once

enum class ClientState {
    Start, Connecting, Connected, Error,
    Sending, Waiting, Closed, Processing
};

enum class ServerState {
    Start, Listening, Listened, Error,
    Waiting, Processing, Sending
};