// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>

typedef VOID (WINAPI* GetSystemTime_t)(LPSYSTEMTIME);
GetSystemTime_t OriginalGetSystemTime = NULL;

VOID WINAPI HookedGetSystemTime(LPSYSTEMTIME lpSystemTime) {
    lpSystemTime->wYear = 2000;
    lpSystemTime->wMonth = 1;
    lpSystemTime->wDay = 1;
    lpSystemTime->wHour = 12;
    lpSystemTime->wMinute = 0;
    lpSystemTime->wSecond = 0;
    lpSystemTime->wMilliseconds = 0;
}

void PatchIAT() {
    HMODULE hModule = GetModuleHandle(NULL);
    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
        hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);

    while (pImportDesc->Name) {
        LPCSTR pszModName = (LPCSTR)((PBYTE)hModule + pImportDesc->Name);
        if (_stricmp(pszModName, "kernel32.dll") == 0) {
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->FirstThunk);
            while (pThunk->u1.Function) {
                PROC* ppfn = (PROC*)&pThunk->u1.Function;
				HMODULE hm = GetModuleHandleA("kernel32.dll");
				if (hm != NULL) {
					if (*ppfn == (PROC)GetProcAddress(hm, "GetSystemTime")) {
						DWORD oldProtect;
						VirtualProtect(ppfn, sizeof(PROC), PAGE_EXECUTE_READWRITE, &oldProtect);
						OriginalGetSystemTime = (GetSystemTime_t)*ppfn;
						*ppfn = (PROC)HookedGetSystemTime;
						VirtualProtect(ppfn, sizeof(PROC), oldProtect, &oldProtect);
						break;
					}
				}
                ++pThunk;
            }
        }
        ++pImportDesc;
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        MessageBoxA(NULL, "DLL Injected Successfully!", "DLL", MB_OK);
        PatchIAT();
    }
    return TRUE;
}

