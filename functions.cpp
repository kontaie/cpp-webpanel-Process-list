#include "functions.hpp"

std::unordered_map<std::string, DWORD> functions::GetProcessList() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return {};
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    std::unordered_map<std::string, DWORD> processes;
    processes.reserve(128); 

    if (Process32FirstW(snap, &pe32)) {
        do {
            processes.insert({utils::wstring_to_utf8(pe32.szExeFile), pe32.th32ProcessID});
        } while (Process32NextW(snap, &pe32));
    }

    CloseHandle(snap);
    return processes;
}

void functions::monitorProcess(
    const std::unordered_map<std::string, DWORD>& processList,
    std::unordered_set<std::string>& blockedList)
{
    for (auto& process : processList) {
        if (blockedList.find(process.first) != blockedList.end()) {
            HANDLE proc = OpenProcess(PROCESS_TERMINATE, FALSE, process.second);
            std::cout << "obtained handle " << proc << std::endl;
            if (proc) { 
                TerminateProcess(proc, 0);
                CloseHandle(proc);
            }
        }
    }
}
