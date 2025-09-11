#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <time.h>

int main()
{
	std::cout << "Inject DLL and press Enter" << std::endl;
	// std::cin.get();
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	std::cout << std::setfill('0') << "GetSystemTime:" << std::setw(2) << systemTime.wYear <<
		"-" << std::setw(2) << systemTime.wMonth <<
		"-" << std::setw(2) << systemTime.wDay <<
		" " << std::setw(2) << systemTime.wHour <<
		":" << std::setw(2) << systemTime.wMinute <<
		":" << std::setw(2) << systemTime.wSecond << std::endl;

    time_t ltime;
    _time64(&ltime);
    std::cout << "_time64:" << ltime << std::endl;
}
