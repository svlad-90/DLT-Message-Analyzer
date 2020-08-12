#ifndef OSHELPER_HPP
#define OSHELPER_HPP

#include <cstdint>

// helper, which is used to wrap operating system dependent functionality

/**
 * @brief getRAMSize - returns back abount of RAM on target machine
 * @return - amount of RAM in Megaytes
 * Note! This function is supported for Windows and Linux.
 */
bool getRAMSize(uint32_t& val);

#endif // OSHELPER_HPP
