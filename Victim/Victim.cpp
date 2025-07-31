// Victim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

class mcolor {
private:
	__int32 r;
	__int32 g;
	__int32 b;
};

struct _u1 {
	union {
		mcolor g;
		__int32 x;
		__int32 y;
		__int32 z;
	} u2;
} u1;

int main()
{
    std::cout << "Victim" << std::endl;

	_u1 ou1;
	ou1.u2.x = 1828;
	ou1.u2.y = 9999;
	ou1.u2.z = 7777;

	mcolor nm = ou1.u2.g;

	DWORD pid;
    std::string dllPath;
    std::cout << "Press Enter";
    std::cin >> pid;
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	std::cout << "day:" << systemTime.wDay << std::endl;
}
