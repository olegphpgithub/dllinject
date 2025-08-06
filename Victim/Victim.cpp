#include <iostream>
#include <Windows.h>

int main()
{
	std::cout << "Inject DLL and press Enter";
	//std::cin.get();
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	std::cout << "day:" << systemTime.wDay << std::endl;
}
