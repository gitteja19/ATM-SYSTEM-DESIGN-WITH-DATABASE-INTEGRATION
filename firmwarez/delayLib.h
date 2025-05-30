#ifndef _DELAYLIB_H
#define _DELAYLIB_H


#include "types.h" // Include custom type definitions

/**
 * @brief Provides a software-based delay in seconds.
 * @param dly The number of seconds to delay.
 */
void delayS(u32);

/**
 * @brief Provides a software-based delay in milliseconds.
 * @param dly The number of milliseconds to delay.
 */
void delayMs(u32 dly);

/**
 * @brief Provides a software-based delay in microseconds.
 * @param dly The number of microseconds to delay.
 */
void delayUs(u32 dly);


#endif
