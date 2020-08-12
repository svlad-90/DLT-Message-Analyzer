#include "OSHelper.hpp"

#ifdef __linux__
    #include <string>
    #include <fstream>
    #include <limits>
#elif _WIN32
    #include <Windows.h>
#endif

const int DIV = 1024;

bool getRAMSize(uint32_t& val)
{
    bool bResult = false;


#ifdef __linux__
    std::string token;
    std::ifstream file("/proc/meminfo");
    while(file >> token) {
        if(token == "MemTotal:") {
            unsigned long mem;
            if(file >> mem)
            {
                bResult = true;
                val = mem/DIV;
            }
            break; // break the loop
        }
        // ignore rest of the line
        file.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
#elif _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    bResult = 0 != GlobalMemoryStatusEx(&statex);

    if(true == bResult)
    {
        val = static_cast<uint32_t>(statex.ullTotalPhys/DIV/DIV);
    }
#endif

    return bResult;
}
