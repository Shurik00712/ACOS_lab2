#pragma once
#include <string>
namespace protocol {
    inline const int kMaxMessageSize = 1024;
    inline const int kMaxConnections = 1;
    inline const std::string kPing = "PING";
    inline const std::string kPong = "PONG";
    inline const std::string kError = "ERROR";
    inline const std::string kHistory       = "HISTORY";
    inline const std::string kQuit          = "QUIT";
    inline const std::string kOk            = "OK";
    inline const std::string kBye           = "BYE";
}