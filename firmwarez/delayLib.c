//software delays library
#include "types.h" // Include custom type definitions

// Constant representing the system clock frequency in Hz for delay calculations.
// Assuming a 12MHz crystal oscillator (FOSC) and a CCLK of FOSC * 5 = 60MHz.
// Each decrement in the while loop might take a few clock cycles, this constant
// is empirically determined for accurate delays.
#define CC 60000000; // Calculated based on a CCLK of 60MHz

/**
 * @brief Provides a software-based delay in seconds.
 *
 * This function creates a blocking delay for the specified number of seconds.
 * The delay is implemented using a simple busy-wait loop.
 *
 * @param dly The number of seconds to delay.
 */
void delayS(u32 dly){
	dly*=12000000; // Adjust delay count based on CPU cycles per second (approx. 12 million cycles/second for 12MHz FOSC loop iterations)
	//dly*=(CC/5); // Alternative calculation based on CC (if loop takes 5 cycles)
	while(dly--); // Busy-wait loop
}

/**
 * @brief Provides a software-based delay in milliseconds.
 *
 * This function creates a blocking delay for the specified number of milliseconds.
 * The delay is implemented using a simple busy-wait loop.
 *
 * @param dly The number of milliseconds to delay.
 */
void delayMs(u32 dly){
	dly*=12000; // Adjust delay count based on CPU cycles per millisecond (approx. 12 thousand cycles/millisecond for 12MHz FOSC loop iterations)
	//dly*=(CC/5000); // Alternative calculation based on CC (if loop takes 5 cycles and needs 5000ms for 1s)
	while(dly--); // Busy-wait loop
}

/**
 * @brief Provides a software-based delay in microseconds.
 *
 * This function creates a blocking delay for the specified number of microseconds.
 * The delay is implemented using a simple busy-wait loop.
 *
 * @param dly The number of microseconds to delay.
 */
void delayUs(u32 dly){
	dly*=12; // Adjust delay count based on CPU cycles per microsecond (approx. 12 cycles/microsecond for 12MHz FOSC loop iterations)
	//dly*=(CC/5000000); // Alternative calculation based on CC (if loop takes 5 cycles and needs 5000000us for 1s)
	while(dly--); // Busy-wait loop
}
