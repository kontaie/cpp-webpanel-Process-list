#pragma once

#include <winsock2.h>
#include <stdexcept>
#include <iostream>

#include "https.hpp"
#include "json.hpp"  

#pragma comment(lib, "ws2_32.lib")

constexpr const char* fHTML = "src.html";
constexpr const char* fCSS = "style.css";
constexpr const char* fJS = "client.js";

namespace utils {
    std::string readFile(const char* path);
    std::string wstring_to_utf8(const std::wstring& str);

    enum class FunctionCode {
        Process,
        block
    };
}

class Server {
public:
    explicit Server();
    ~Server();

private:
    void run();
    void handleRequest(HTTP& https, const std::string& body);
    nlohmann::json handleResponse(utils::FunctionCode fCode, std::string* command);


    SOCKET clientSocket = INVALID_SOCKET;
    SOCKET serverSocket = INVALID_SOCKET;
    WSAData ws{};
};
