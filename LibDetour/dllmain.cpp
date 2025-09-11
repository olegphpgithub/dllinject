// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <iostream>

#include <detours.h>
#pragma comment(lib, "detours.lib")

SYSTEMTIME spurious_systime;

static VOID (WINAPI* TrueGetSystemTime)(LPSYSTEMTIME) = GetSystemTime;
static VOID (WINAPI* TrueGetSystemTimeAsFileTime)(LPFILETIME) = GetSystemTimeAsFileTime;
static VOID (WINAPI* TrueGetLocalTime)(LPSYSTEMTIME) = GetLocalTime;

typedef VOID (WINAPI* GetSystemTime_t)(LPSYSTEMTIME);
GetSystemTime_t OriginalGetSystemTime = NULL;

VOID WINAPI HookedGetSystemTime(LPSYSTEMTIME lpSystemTime) {
    *lpSystemTime = spurious_systime;
}

VOID WINAPI HookedGetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime) {
    SystemTimeToFileTime(&spurious_systime, lpSystemTimeAsFileTime);
}

VOID WINAPI HookedGetLocalTime(LPSYSTEMTIME lpSystemTime) {
    *lpSystemTime = spurious_systime;
}

__time64_t SystemTimeTo_time64(const SYSTEMTIME& st)
{
    FILETIME ft;
    if (!SystemTimeToFileTime(&st, &ft)) {
        return -1;
    }

    ULARGE_INTEGER ull;
    ull.LowPart  = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    static const ULONGLONG EPOCH_DIFF = 116444736000000000ULL;

    return (__time64_t)((ull.QuadPart - EPOCH_DIFF) / 10000000ULL);
}


typedef __time64_t (__cdecl* time64_t_fn)(__time64_t*);
static time64_t_fn True_time64 = nullptr;

__time64_t __cdecl Hooked_time64(__time64_t* t) {
    __time64_t fake = SystemTimeTo_time64(spurious_systime);
    if (t) {
        *t = fake;
    }
    return fake;
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
        //DisableThreadLibraryCalls(hModule);

        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourAttach(&(PVOID&)TrueGetSystemTimeAsFileTime, HookedGetSystemTimeAsFileTime);
        DetourAttach(&(PVOID&)TrueGetLocalTime, HookedGetLocalTime);

        HMODULE hCRT = LoadLibraryA("msvcrt.dll");
        if (hCRT) {
            True_time64 = (time64_t_fn)GetProcAddress(hCRT, "_time64");
            DetourAttach(&(PVOID&)True_time64, Hooked_time64);
        }

        DetourTransactionCommit();
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueGetSystemTime, HookedGetSystemTime);
        DetourDetach(&(PVOID&)TrueGetSystemTimeAsFileTime, HookedGetSystemTimeAsFileTime);
        DetourDetach(&(PVOID&)TrueGetLocalTime, HookedGetLocalTime);

        if (True_time64) {
            DetourDetach(&(PVOID&)True_time64, Hooked_time64);
        }

        DetourTransactionCommit();
    }
    return TRUE;
}
