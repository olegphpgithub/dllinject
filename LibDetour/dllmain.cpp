// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#include <detours.h>
#pragma comment(lib, "detours.lib")

SYSTEMTIME spurious_systime;

static VOID (WINAPI* TrueGetSystemTime)(LPSYSTEMTIME) = GetSystemTime;

typedef VOID (WINAPI* GetSystemTime_t)(LPSYSTEMTIME);
GetSystemTime_t OriginalGetSystemTime = NULL;

VOID WINAPI HookedGetSystemTime(LPSYSTEMTIME lpSystemTime) {
    *lpSystemTime = spurious_systime;
    
    //lpSystemTime->wYear = spurious_systime.wYear;
    //lpSystemTime->wMonth = spurious_systime.wMonth;
    //lpSystemTime->wDay = spurious_systime.wDay;
    //lpSystemTime->wHour = 12;
    //lpSystemTime->wMinute = 0;
    //lpSystemTime->wSecond = 0;
    //lpSystemTime->wMilliseconds = 0;
}

extern "C" __declspec(dllexport) void AssignSystemTime(const SYSTEMTIME *systime) {
    std::cout << "From DLL:" << systime->wYear << std::endl;
    spurious_systime = *systime;
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
