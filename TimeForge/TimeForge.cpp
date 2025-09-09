// TimeForge.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "CommandLineParser.h"
#include "CLI11.hpp"

CommandLineParser parser;

bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    LPVOID allocAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocAddr) return false;

    if (!WriteProcessMemory(hProcess, allocAddr, dllPath, strlen(dllPath) + 1, NULL))
        return false;

	HMODULE hModule = GetModuleHandleA("kernel32.dll");
	if (hModule == NULL) {
		return false;
	}

    LPVOID loadLibraryAddr = GetProcAddress(hModule, "LoadLibraryA");
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



bool PassParameter(DWORD pid, const char* dllPath)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) return false;

	// Получаем базовый адрес загруженной DLL (в ЦЕЛЕВОМ процессе)
    HMODULE hMods[1024];
    DWORD cbNeeded;
    HMODULE injectedBase = NULL;

    if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            char modName[MAX_PATH];
            GetModuleFileNameExA(hProcess, hMods[i], modName, sizeof(modName));
            if (_stricmp(strrchr(modName, '\\') + 1, strrchr(dllPath, '\\') + 1) == 0) {
                injectedBase = hMods[i];
                break;
            }
        }
    }

    if (!injectedBase) {
        std::cerr << "DLL not found in remote process\n";
        return false;
    }

    // 2. Вычисляем RVA (смещение) экспортируемой функции Init
    HMODULE hLocalDLL = LoadLibraryA(dllPath); // Загрузить локально для анализа
    FARPROC pInitLocal = GetProcAddress(hLocalDLL, "AssignSystemTime");

    DWORD_PTR offset = (DWORD_PTR)pInitLocal - (DWORD_PTR)hLocalDLL;
    DWORD_PTR remoteInit = (DWORD_PTR)injectedBase + offset;

    FreeLibrary(hLocalDLL); // больше не нужен

    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, sizeof(parser.m_time), MEM_COMMIT, PAGE_READWRITE);
    if (remoteMem == NULL) return false;

	WriteProcessMemory(hProcess, remoteMem, &(parser.m_time), sizeof(parser.m_time), NULL);

    // 4. Вызываем функцию Init с параметром
    HANDLE hInitThread = CreateRemoteThread(hProcess, NULL, 0,
                                            (LPTHREAD_START_ROUTINE)remoteInit,
                                            remoteMem, 0, NULL);
    WaitForSingleObject(hInitThread, INFINITE);

    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hInitThread);
    CloseHandle(hProcess);
	return true;
}

int main(int argc, char *argv[]) {

    if (!parser.Parse(argc, argv))
    {
        exit(1);
    }

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    // LPCSTR appPath = "d:\\nnRus.Git\\dllinject\\Release\\Victim.exe";

    std::stringstream cmdlinestream;

    std::string border = parser.target_application.find(' ') != std::string::npos ? "\"" : "";
    cmdlinestream << border << parser.target_application << border << " " << parser.command_line;
    std::string cmdlinestr = cmdlinestream.str();

    LPCSTR appPath = parser.target_application.c_str();
    char *cmd = NULL;
    cmd = new char[cmdlinestr.length() + 1];
    strncpy_s(cmd, cmdlinestr.length() + 1, cmdlinestr.c_str(), cmdlinestr.length());
    cmd[cmdlinestr.length()] = 0;

    BOOL success = CreateProcessA(
        appPath,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Process has been launched in sus (PID: " << pi.dwProcessId << ")\n";


    if (InjectDLL(pi.dwProcessId, "d:\\nnRus.Git\\dllinject\\Release\\LibDetour.dll"))
        std::cout << "DLL successfully injected!\n";
    else
        std::cout << "Injection failed.\n";

	if (PassParameter(pi.dwProcessId, "d:\\nnRus.Git\\dllinject\\Release\\LibDetour.dll")) {
		std::cout << "DLL passed ok\n";
	}
	else {
		std::cout << "DLL passed fail\n";
	}

    ResumeThread(pi.hThread);

    std::cout << "Proc resumed.\n";

	DWORD dw = WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}
