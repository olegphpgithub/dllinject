// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>

#include <detours.h>
#pragma comment(lib, "detours.lib")


static VOID (WINAPI* TrueGetSystemTime)(LPSYSTEMTIME) = GetSystemTime;

typedef VOID (WINAPI* GetSystemTime_t)(LPSYSTEMTIME);
GetSystemTime_t OriginalGetSystemTime = NULL;

VOID WINAPI HookedGetSystemTime(LPSYSTEMTIME lpSystemTime) {
    lpSystemTime->wYear = 2000;
    lpSystemTime->wMonth = 5;
    lpSystemTime->wDay = 9;
    lpSystemTime->wHour = 12;
    lpSystemTime->wMinute = 0;
    lpSystemTime->wSecond = 0;
    lpSystemTime->wMilliseconds = 0;
}

extern "C" __declspec(dllexport) void DummyExport(const char* dllPath) {
	printf("From Dll: %s\n", dllPath);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        MessageBoxA(NULL, "DLL Injected with Detours!", "Detours Hook", MB_OK);

        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourTransactionCommit();
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourTransactionCommit();
    }
    return TRUE;
}
