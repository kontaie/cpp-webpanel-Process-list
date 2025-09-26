#pragma once

#include "server.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <windows.h>   
#include <TlHelp32.h>
#include <unordered_set>

namespace functions {
    std::unordered_map<std::string, DWORD> GetProcessList();
    void monitorProcess(
        const std::unordered_map<std::string, DWORD>& processList,
        std::unordered_set<std::string>& blockedList);
}
