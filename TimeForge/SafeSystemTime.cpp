#include "SafeSystemTime.h"

#include <Windows.h>

SafeSystemTime::SafeSystemTime(WORD year, WORD month, WORD day, 
                WORD hour, WORD minute, 
                WORD second, WORD milliseconds) {
    m_time.wYear = year;
    m_time.wMonth = month;
    m_time.wDay = day;
    m_time.wHour = hour;
    m_time.wMinute = minute;
    m_time.wSecond = second;
    m_time.wMilliseconds = milliseconds;
}

bool SafeSystemTime::isValid() const {
    FILETIME ft;
    return SystemTimeToFileTime(&m_time, &ft);
}

SYSTEMTIME SafeSystemTime::getSystemTime()
{
    return m_time;
}
