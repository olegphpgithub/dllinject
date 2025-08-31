#pragma once

#include <Windows.h>

class SafeSystemTime {
private:
    SYSTEMTIME m_time;

public:
    SafeSystemTime(WORD year, WORD month, WORD day, 
                  WORD hour = 0, WORD minute = 0, 
                  WORD second = 0, WORD milliseconds = 0);
    bool isValid() const;
    SYSTEMTIME getSystemTime();

};
