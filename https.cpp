#include "https.hpp"
#include <format>
#include <stdexcept>
#include <iostream>

HTTP::HTTP(SOCKET clientSocket) : clientSocket(std::move(clientSocket)) {}

void HTTP::HTTP_POST(int status, const char* mime, const std::string& body) {
    auto header = std::format(
        "HTTP/1.1 {} OK\r\n"
        "Content-Type: {}\r\n"
        "Content-Length: {}\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n",
        status, mime, body.size()
    );

    send_req(header + body);

    std::cout << "success {"
        << std::string(body.begin(), body.begin() + 40) + "\n}"
        << std::endl;
}

void HTTP::send_req(const std::string& data) {
    int result = send(clientSocket, data.c_str(), (int)data.size(), 0);
    if (result == SOCKET_ERROR) {
        throw std::runtime_error("Failed to send request");
    }
}
