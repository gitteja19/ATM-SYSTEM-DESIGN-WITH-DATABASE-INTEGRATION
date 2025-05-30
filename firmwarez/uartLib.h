#ifndef _UARTLIB_H_
#define _UARTLIB_H_

#include "types.h" // Include custom type definitions

// Clock frequency definitions for baud rate calculation
#define FOSC 12000000       // External Oscillator Frequency (12 MHz)
#define CCLK (FOSC*5)       // CPU Clock Frequency (60 MHz, assuming PLL multiplier M=5)
#define PCLK (CCLK/4)       // Peripheral Clock Frequency (15 MHz, assuming VPBDIV=01 for PCLK = CCLK/4)
#define BAUD 9600           // Desired Baud Rate (9600 bps)

// Divisor Latch Value for baud rate generation.
// DVSR = PCLK / (16 * BAUD)
#define DVSR (PCLK/(16*BAUD))

// UART unit definitions
#define U0 0 // Represents UART0
#define U1 1 // Represents UART1

// Pin definitions for UART Transmit (TX) and Receive (RX)
// These refer to the bit positions in PINSEL0 register for LPC214x.
#define U0TX 0  // P0.0 for UART0 TX
#define U0RX 1  // P0.1 for UART0 RX
#define U1TX 8  // P0.8 for UART1 TX
#define U1RX 9  // P0.9 for UART1 RX

// Special Function Register (SFR) bit numbers
// UxLCR (UART Line Control Register) bits
#define UART_DLAB	7   // Divisor Latch Access Bit (DLAB): Enables access to DLL/DLM
#define UART_WRDLEN 3   // Word Length Select: Set to 3 (binary 11) for 8-bit word length

// UxLSR (UART Line Status Register) bits
#define UART_TEMT	6   // Transmitter Empty: Indicates THR and TSR are empty
#define UART_DR		0   // Data Ready: Indicates data is available in RBR

// Function declarations for UART operations

/**
 * @brief Initializes the specified UART unit (UART0 or UART1).
 * Configures pin selection, baud rate, and 8-bit word length.
 * @param un The UART unit number (0 for UART0, 1 for UART1).
 */
void initUART(u8 un);

/**
 * @brief Transmits a single byte via the specified UART unit.
 * Waits until the transmit buffer is empty before sending.
 * @param un The UART unit number.
 * @param word The 8-bit data to transmit.
 */
void txUART(u8 un,u8 word);

/**
 * @brief Receives a single byte from the specified UART unit.
 * Checks for data availability and returns the byte if ready. Non-blocking.
 * @param un The UART unit number.
 * @return The received 8-bit data, or 0 if no data is ready.
 */
u8	 rxUART(u8); // Declared but not defined in uartLib.c - typically a blocking read
u8 	 rx_UART(u8 un); // Defined in uartLib.c - non-blocking read

// Function declarations for transmitting various data types via UART
void charTxUART(u8,s8 );  // Transmit a single character
void strTxUART(u8,s8*);   // Transmit a null-terminated string
void intTxUART(u8,s32);   // Transmit a signed integer
void uintTxUART(u8,u32);  // Transmit an unsigned integer
void fltTxUART(u8,f32);   // Transmit a float

// Function declarations for receiving various data types via UART
s8  charRxUART(u8);   // Receive a single character
void strRxUART(u8,s8*); // Receive a string
s32  intRxUART(u8);   // Receive a signed integer
u32 uintRxUART(u8);   // Receive an unsigned integer
f32  fltRxUART(u8);   // Receive a float

/**
 * @brief Receives a string from the specified UART unit until a newline character.
 * Stores the received characters in the provided buffer and null-terminates it.
 * @param un The UART unit number.
 * @param buf A pointer to the buffer to store the received string.
 */
void strRx_UART(u8 un,s8* buf);

#endif
