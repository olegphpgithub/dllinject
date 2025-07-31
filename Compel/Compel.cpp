// Compel.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h>


bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    LPVOID allocAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocAddr) return false;

    if (!WriteProcessMemory(hProcess, allocAddr, dllPath, strlen(dllPath) + 1, NULL))
        return false;

    LPVOID loadLibraryAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddr) return false;

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)loadLibraryAddr,
                                        allocAddr, 0, NULL);
    if (!hThread) return false;

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, allocAddr, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}
int main()
{
    DWORD pid;
    std::string dllPath;
    std::cout << "Enter target process PID: ";
    std::cin >> pid;
    std::cout << "Enter full path to DLL: ";
    std::cin >> dllPath;

    if (InjectDLL(pid, dllPath.c_str()))
        std::cout << "DLL successfully injected!\n";
    else
        std::cout << "Injection failed.\n";

    return 0;
}
