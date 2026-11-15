#pragma once
#include <string>
namespace protocol {
    const int kMaxMessageSize = 1024;
    const std::string kPing = "PING";
    const std::string kPong = "PONG";
    const std::string kError = "ERROR";
}