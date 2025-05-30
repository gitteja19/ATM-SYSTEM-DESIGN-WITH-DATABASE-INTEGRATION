// atmLib.h - Header file for ATM functionalities and configurations
#ifndef _ATMLIB_H
#define _ATMLIB_H

// Include standard microcontroller and system headers
#include <lpc214x.h>  // Specific header for NXP LPC214x microcontroller peripherals
#include <string.h>   // String manipulation functions (e.g., strlen, strcmp)
#include <stdio.h>    // Standard input/output functions (e.g., sprintf)
#include <stdlib.h>   // Standard library functions (e.g., for general utilities)

// Include custom peripheral libraries
#include "types.h"    // Custom data type definitions (u8, s32, etc.)
#include "kpmLib.h"   // Keypad Matrix Library
#include "lcdLib.h"   // LCD Display Library
#include "uartLib.h"  // UART Communication Library
#include "delayLib.h" // Software Delay Library
#include "rfidLib.h"  // RFID Reader Library
// #include "atmLib.h" // Self-inclusion - usually not needed in a header, might indicate nested inclusion or a copy-paste error.

// Conditional Compilation Macro for UART Interrupt
#define UART_INT // Define this to enable UART interrupt-driven reception.
                 // If commented out, polling-based reception will be used.

// Macro for casting to const char* (convenience for string literals)
#define CC (const char*)

// ATM operational limits and configurations
#define MAX_TRYS 		3       // Maximum number of PIN retries allowed
#define MAX_DEPOSIT     30000   // Maximum amount allowed for a single deposit (30,000 Rs)
#define MAX_WITHDRAW    30000   // Maximum amount allowed for a single withdrawal (30,000 Rs)
#define MAX_TRANSFER    100000  // Maximum amount allowed for a single transfer (100,000 Rs)


// Input buffer configurations for keypad and string input
#define BUF_MAX 50  // Maximum size of the generic communication buffer (e.g., for UART messages, keypad input)
#define PIN 1       // Identifier for PIN input mode (used internally or by related functions)
#define STR 0       // Identifier for general string input mode

// Timing definitions
#define DISP_TIME 1000 	      // Display message duration in milliseconds (1 second)
#define ATM_TIME  (30*120000) // Session timeout for ATM in arbitrary units (e.g., 30 seconds * some delay loop constant)


// External declarations for global variables (defined in atmLib.c)
extern volatile s8 buf[],dummy;           // 'buf' for communication, 'dummy' for clearing interrupt flags
extern volatile u32 r_flag,time,buf_index; // 'r_flag' for UART receive status, 'time' for session timeout, 'buf_index' for buffer management

// Function declarations for system initialization and core ATM functionalities

/**
 * @brief Initializes all necessary hardware peripherals (UART, LCD, Keypad).
 */
void sys_init(void);

/**
 * @brief Interrupt Service Routine (ISR) for UART0.
 * Handles incoming data from the PC/server.
 */
void UART0_isr(void) __irq;

/**
 * @brief Checks and maintains the communication line with the PC/server.
 * Ensures the PC is responsive before proceeding with transactions.
 */
void checkPC(void);

/**
 * @brief Sends a null-terminated string message to the PC/server via UART0.
 * @param str Pointer to the string to be sent.
 */
void sendMsg(s8 *str);

/**
 * @brief Receives a null-terminated string message from the PC/server via UART0.
 * @param str Pointer to the buffer where the received string will be stored.
 */
void getMsg( s8 *str);

/**
 * @brief Checks if a received message from the PC/server is in a valid format.
 * (e.g., starts with '@' and ends with '$').
 * @param str Pointer to the message string to check.
 * @return 1 if the message is valid, 0 otherwise.
 */
s32  isMsgOk(s8 *str);

/**
 * @brief Prompts the user to enter a 4-digit PIN using the keypad.
 * Displays '*' for entered digits for security and handles backspace/cancel.
 * Implements a session timeout.
 * @param str Pointer to the buffer where the entered PIN will be stored.
 * @return 1 if PIN entered successfully, 0 if timed out, -1 if cancelled.
 */
u32  getPin( s8 *str);

/**
 * @brief Reads a general string input from the keypad and displays it on the LCD.
 * Handles numeric input, backspace, and confirmation keys.
 * Implements a session timeout.
 * @param str Pointer to the buffer for storing the input string.
 * @param row The LCD row to display the input.
 * @return 0 for valid string input, 1 for timeout, -1 for cancellation.
 */
int str_KPM( s8 *str,u8 row);

// Function declarations for main ATM transaction operations

/**
 * @brief Handles the "Withdraw Cash" transaction.
 * Prompts for amount, validates it, and communicates with the PC for processing.
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to the communication buffer.
 */
void atm_wtd(s8 *rfid,s8 *buf);

/**
 * @brief Handles the "Deposit Cash" transaction.
 * Prompts for amount and communicates with the PC for processing.
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to the communication buffer.
 */
void atm_dep(s8 *rfid,s8 *buf);

/**
 * @brief Handles the "View Balance" inquiry.
 * Communicates with the PC to retrieve and display the account balance.
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to the communication buffer.
 */
void atm_bal(s8 *rfid,s8 *buf);

/**
 * @brief Handles the "Mini Statement" request.
 * Retrieves and displays recent transactions from the PC.
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to the communication buffer.
 */
void atm_mst(s8 *rfid,s8 *buf);

/**
 * @brief Handles the "PIN Change" functionality.
 * Guides the user through entering old and new PINs, and updates the PIN on the PC.
 * @param rfid Pointer to the RFID tag number string.
 * @param pin Pointer to the current PIN string (will be updated on success).
 * @param buf Pointer to the communication buffer.
 * @return 1 if PIN change is successful, 0 otherwise.
 */
int  atm_pin(s8 *rfid,s8 *pin,s8 *buf);


#endif
