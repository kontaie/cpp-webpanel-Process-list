#pragma once


#include <winsock2.h>
#include <string>

class HTTP {
public:
    explicit HTTP(SOCKET clientSocket);

    void HTTP_POST(int status, const char* mime, const std::string& body);

private:
    void send_req(const std::string& data);

    SOCKET clientSocket;
};
