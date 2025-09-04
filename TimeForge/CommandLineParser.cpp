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
#include "CLI11.hpp"

bool CommandLineParser::Parse(int argc, char* argv[])
{
    CLI::App app{"Example with CLI11"};
    app.allow_extras();

    app.add_flag("--datetime", target_datetime, "Datetime");
    app.add_flag("--application", target_application, "Application");
    bool parameters_mode = false;
    app.add_flag("--parameters", parameters_mode, "Parameters");

    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> names;
    if (parameters_mode) {
        for (auto &arg : app.remaining()) {
            names.push_back(arg);
        }
    }

    std::cout << "App1908:" << target_application << "\n";

    std::stringstream command_line_stream;
    std::string delimiter("");
    std::string border("");
    for (const std::string &n : names) {
        delimiter = command_line_stream.str().empty() ? "" : " ";
        border = n.find(' ') != std::string::npos ? "\"" : "";
        command_line_stream << delimiter << border << n << border;
    }
    command_line = command_line_stream.str();

    std::cout << "cl:" << command_line << "\n";

    /*
    const char* const short_opts = "a:p:d:";
    const option long_opts[] = {
            {"application", required_argument, nullptr, 'a'},
            {"parameter", required_argument, nullptr, 'p'},
            {"datetime", required_argument, nullptr, 'd'},
            {"help", no_argument, nullptr, 'h'},
            {nullptr, no_argument, nullptr, 0}
    };

    std::ostringstream command_line_stream;

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt) {
            break;
        }

        std::string border("");

        switch (opt)
        {
        case 'a':
            target_application = std::string(optarg);
            break;

        case 'p':
            if (!command_line_stream.str().empty()) {
                command_line_stream << " ";
            }

            if (strchr(optarg, ' ')) {
                border = "\"";
            }
            else {
                border = "";
            }

            command_line_stream << border << std::string(optarg) << border;
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
    command_line = command_line_stream.str();
    */

    return ValidateArguments();
}

bool CommandLineParser::ValidateArguments()
{
    std::cout << "command_line:" << command_line.c_str() << std::endl;

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

                m_time = sst.getSystemTime();
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
