#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 

#include "server.hpp"
#include "https.hpp"
#include "functions.hpp"
#include "json.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <codecvt>
#include <optional>
#include <unordered_map>
#include <functional>
#include <thread>

using namespace nlohmann;
std::unordered_set<std::string> blocked;

namespace utils {

    std::string readFile(const char* path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "Could not open file: " << path << "\n";
            return {};
        }
        return { std::istreambuf_iterator<char>(file),
                 std::istreambuf_iterator<char>() };
    }

    std::string wstring_to_utf8(const std::wstring& str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(str);
    }

    std::optional<json> safeParse(const std::string& s) {
        try {
            return json::parse(s);
        }
        catch (...) {
            return std::nullopt;
        }
    }

} 


// ===================== Server Implementation =====================

Server::Server() {
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        throw std::runtime_error("Invalid server socket");
    }

    SOCKADDR_IN sockAddr{};
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(8080);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr)) != 0) {
        throw std::runtime_error("Failed to bind");
    }

    std::cout << "Server running at http://localhost:8080\n";

    std::thread([]() {
        while (true) {
            auto processes = functions::GetProcessList();
            functions::monitorProcess(processes, blocked);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        }).detach();

    run();
}


Server::~Server() {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    if (serverSocket != INVALID_SOCKET) closesocket(serverSocket);
    WSACleanup();
}

void Server::run() {
    if (listen(serverSocket, SOMAXCONN) != 0) {
        throw std::runtime_error("Failed to listen");
    }

    std::cout << "Listening for clients...\n";

    while (true) {
        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            throw std::runtime_error("Invalid client socket");
        }

        try {
            std::vector<char> buffer(8192);
            int received = recv(clientSocket, buffer.data(), buffer.size(), 0);
            if (received > 0) {
                HTTP https(clientSocket);
                std::string data(buffer.data(), received);
                std::cout << data << std::endl;
                handleRequest(https, data);
            }
        }
        catch (const std::exception& ex) {
            std::cerr << ex.what() << std::endl;
        }
    }
}

json Server::handleResponse(utils::FunctionCode fCode, std::string* command) {
    json res;
    auto processes = functions::GetProcessList();
    std::unordered_set<std::string> processNames;

    for (auto& process : processes) {
        processNames.insert(process.first);
    }

    switch (fCode) {
    case utils::FunctionCode::Process: {
        res["process"] = processNames;
        res["status"] = "ok";
        break;
    }
    case utils::FunctionCode::block: {
        if (command) {
            std::stringstream ss(*command);
            std::string appname;
            ss >> appname;

            if (processNames.find(appname) != processNames.end()) {
                std::cout << "inserted " << appname << std::endl;
                blocked.insert(appname);
            }
            else {
                std::cout << "process not found: " << appname << std::endl;
            }
        }
        break;
    }
    default: break;
    }

    return res;
}

void Server::handleRequest(HTTP& https, const std::string& body) {
    auto sendFile = [&](const std::string& path, const char* mime) {
        std::string file = utils::readFile(path.c_str());
        if (!file.empty()) {
            https.HTTP_POST(200, mime, file);
        }
    };

    const std::vector<std::tuple<std::string, const char*, const char*>> routes = {
        { "GET / HTTP",        fHTML, "text/html" },
        { "GET /style.css",    fCSS,  "text/css" },
        { "GET /client.js",    fJS,   "application/javascript" }
    };

    for (const auto& [pattern, path, mime] : routes) {
        if (body.find(pattern) != std::string::npos) {
            return sendFile(path, mime);
        }
    }

    auto pos = body.find("\r\n\r\n");
    if (pos == std::string::npos) return;

    std::string jsonBody = body.substr(pos + 4);
    auto j = utils::safeParse(jsonBody);
    if (!j) {
        std::cout << "error parsing" << std::endl;
        return;
    }

    using FC = utils::FunctionCode;
    std::string command = j->value("command", "");

    using Handler = std::function<void(const std::string&)>;

    std::unordered_map<std::string, Handler> cmdHandlers = {
        { "apps", [&](const std::string&) {
            auto res = handleResponse(FC::Process, nullptr);
            https.HTTP_POST(200, "application/json", res.dump(4));
        }},
        { "blocked", [&](std::string cmd) {
            handleResponse(FC::block, &cmd);
        }}
    };

    for (auto& [key, handler] : cmdHandlers) {
        if (command.find(key) != std::string::npos) {
            handler(command);
            break;
        }
    }
}
