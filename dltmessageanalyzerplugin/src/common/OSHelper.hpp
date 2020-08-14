#ifndef OSHELPER_HPP
#define OSHELPER_HPP

#include <cstdint>

// helper, which is used to wrap operating system dependent functionality

/**
 * @brief getRAMSize - returns back abount of RAM ( in Mb ) on the target machine
 * @return - amount of RAM in Megaytes
 * Note! This function is supported for Windows and Linux.
 */
bool getRAMSize(uint32_t& val);

/**
 * @brief getRAMSizeUnchecked - will return RAM size ( in MB ) on the target machine.
 * Does not provide bool result of operation.
 * @return - RAM size in case of success or 0 Otherwise.
 */
uint32_t getRAMSizeUnchecked();

#endif // OSHELPER_HPP
