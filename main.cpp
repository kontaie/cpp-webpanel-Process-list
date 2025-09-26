#include "server.hpp"
#include <Windows.h>
#include <shobjidl_core.h>
#include <iostream>

using namespace std;

#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main() {
    try {
        MessageBoxA(NULL, "Server up and running, site : http://localhost:8080", "Server", MB_OK);
        Server server;  
    }
    catch (exception& ex) {
        MessageBoxA(NULL, ex.what(), "Error", MB_OK | MB_ICONERROR);
    }
    return 0;
}
