#include <iostream>
#include <string>
#include <Windows.h>

#include "resource.h"
#include "ResourceFile.h"

ResourceFile::ResourceFile()
{
    try {
        HMODULE hModule = GetModuleHandle(NULL);

        char tempPath[MAX_PATH];
        if (!GetTempPathA(MAX_PATH, tempPath)) {
            throw std::exception();
        }

        char tempFile[MAX_PATH];
        if (!GetTempFileNameA(tempPath, "DLL", 0, tempFile)) {
            throw std::exception();
        }

        file_path.assign(tempFile);

        HRSRC hRes = FindResourceA(hModule, MAKEINTRESOURCEA(IDR_321), "LIB");
        if (!hRes) {
            throw std::exception();
        }

        DWORD resSize = SizeofResource(hModule, hRes);
        if (resSize == 0) {
            throw std::exception();
        }

        HGLOBAL hResLoad = LoadResource(hModule, hRes);
        if (!hResLoad) {
            throw std::exception();
        }

        void* pResData = LockResource(hResLoad);
        if (!pResData) {
            throw std::exception();
        }

        HANDLE hFile = CreateFileA(file_path.c_str(),
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw std::exception();
        }

        DWORD written = 0;
        BOOL ok = WriteFile(hFile, pResData, static_cast<DWORD>(resSize), &written, NULL);
        if (ok == FALSE) {
            throw std::exception();
        }
        if (written != resSize) {
            throw std::exception();
        }
        CloseHandle(hFile);
    }
    catch (std::exception) {
        throw std::exception("Failed to extract DLL");
    }
}

ResourceFile::~ResourceFile()
{
    DeleteFileA(file_path.c_str());
}
