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
static VOID (WINAPI* TrueGetSystemTimeAsFileTime)(LPFILETIME) = GetSystemTimeAsFileTime;

typedef VOID (WINAPI* GetSystemTime_t)(LPSYSTEMTIME);
GetSystemTime_t OriginalGetSystemTime = NULL;

VOID WINAPI HookedGetSystemTime(LPSYSTEMTIME lpSystemTime) {
    *lpSystemTime = spurious_systime;
}

VOID WINAPI HookedGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
    SystemTimeToFileTime(&spurious_systime, lpSystemTimeAsFileTime);
}

extern "C" __declspec(dllexport) void AssignSystemTime(const SYSTEMTIME *systime) {
    spurious_systime = *systime;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourAttach(&(PVOID&)TrueGetSystemTimeAsFileTime, HookedGetSystemTimeAsFileTime);
        DetourTransactionCommit();
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourDetach(&(PVOID&)TrueGetSystemTimeAsFileTime, HookedGetSystemTimeAsFileTime);
        DetourTransactionCommit();
    }
    return TRUE;
}
