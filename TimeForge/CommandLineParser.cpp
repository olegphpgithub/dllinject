#include "CommandLineParser.h"
#include "SafeSystemTime.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _MSC_VER
    #include <regex>  // MSVC
#else
    #include <tr1/regex>  // GCC, Clang
#endif

#include "getopt.h"

bool CommandLineParser::Parse(int argc, char* argv[])
{
    const char* const short_opts = "a:d:";
    const option long_opts[] = {
            {"application", required_argument, nullptr, 'a'},
            {"datetime", required_argument, nullptr, 'd'},
            {"help", no_argument, nullptr, 'h'},
            {nullptr, no_argument, nullptr, 0}
    };

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt) {
            break;
        }

        switch (opt)
        {
        case 'a':
            target_application = std::string(optarg);
            break;

        case 'd':
            target_datetime = std::string(optarg);
            break;

        case 'h':
        case '?':
        default:
            PrintHelp();
            return false;
        }
    }
    return ValidateArguments();
}

bool CommandLineParser::ValidateArguments()
{
    if (target_application.empty() || target_datetime.empty()) {
        PrintHelp();
        return false;
    }

    try {
        std::ifstream file(target_application);
        if (file) {
            file.close();
        }
        else {
            throw std::exception("File not found.");
            return false;
        }

        try {
            std::regex re(R"((\d{4})-(\d{2})-(\d{2})T\d{2}:\d{2}:\d{2})");
            std::smatch matches;
            if (std::regex_search(target_datetime, matches, re)) {

                wYear = StringToWord(matches[1].str().c_str());
                wMonth = StringToWord(matches[2].str().c_str());
                wDay = StringToWord(matches[3].str().c_str());

                SafeSystemTime sst(wYear, wMonth, wDay);
                if (!sst.isValid()) {
                    throw std::exception("DateTime is invalid.");
                }
            }
            else {
                throw std::exception("String doesn't match regex 0000-00-00T00:00:00.");
            }
        }
        catch (const std::regex_error& e) {
            std::cerr << "Regex error: " << e.what() << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Input validation failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

WORD CommandLineParser::StringToWord(const char* str) {
    std::istringstream iss(str);
    WORD result = 0;

    if (!(iss >> result)) {
        throw std::invalid_argument("Unable to convert value to WORD.");
    }

    char remaining;
    if (iss >> remaining) {
        throw std::invalid_argument("String contains unexpected characters.");
    }

    return result;
}

void CommandLineParser::PrintHelp()
{
    std::cout <<
        "--application <path>\n"
        "--datetime <0000-00-00T00:00:00>\n"
        "--help\n";
}
