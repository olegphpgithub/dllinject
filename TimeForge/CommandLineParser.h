#pragma once

#include <string>
#include <Windows.h>

class CommandLineParser
{
public:
    WORD wYear;
    WORD wMonth;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;

    std::string target_application;
    std::string target_datetime;
    bool Parse(int argc, char* argv[]);
    bool ValidateArguments();
    WORD StringToWord(const char* str);
    void PrintHelp();
};

