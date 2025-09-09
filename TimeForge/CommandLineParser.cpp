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

#include "CLI11.hpp"

bool CommandLineParser::Parse(int argc, char* argv[])
{
    CLI::App app{"CLI11"};
    app.allow_extras();

    app.add_flag("--datetime", target_datetime, "Datetime");

    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> parameters;

    for (auto &arg : app.remaining()) {
        if (target_application.empty()) {
            target_application = std::string(arg);
        }
        else {
            parameters.push_back(arg);
        }
    }

    std::stringstream command_line_stream;
    std::string delimiter("");
    std::string border("");
    for (const std::string &p : parameters) {
        delimiter = command_line_stream.str().empty() ? "" : " ";
        border = p.find(' ') != std::string::npos ? "\"" : "";
        command_line_stream << delimiter << border << p << border;
    }
    command_line = command_line_stream.str();

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
    std::cout << "--datetime=<0000-00-00T00:00:00> <path to the application> <parameters>" << std::endl;
}
