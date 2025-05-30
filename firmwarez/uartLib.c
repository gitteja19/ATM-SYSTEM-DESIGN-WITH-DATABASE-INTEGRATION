#include <lpc214x.h> // Include LPC214x specific header for Special Function Registers (SFRs)
#include "types.h"   // Custom type definitions
#include "uartLib.h" // UART library header
#include "atmLib.h"

/**
 * @brief Initializes the specified UART unit (UART0 and UART1) for communication.
 *
 * This function configures the necessary PINSEL registers for UART TX/RX pins,
 * sets the Line Control Register (LCR) for 8-bit word length and enables DLAB
 * to access Divisor Latch Registers (DLL/DLM) for baud rate setting.
 * Finally, DLAB is cleared to enable normal operation.
 *
 */
void initUART(void){
		// Configure P0.8 (U1TX) and P0.9 (U1RX) for UART1 function.
		// PINSEL0 controls P0.0-P0.15. Each pin has 2 bits.
		// U1TX is P0.8, so its bits are (8*2) = 16 and 17.
		// U1RX is P0.9, so its bits are (9*2) = 18 and 19.
		// (5<<U1TX*2) sets P0.8 and P0.9 to Function 01 (UART1 Tx/Rx).
		PINSEL0=((PINSEL0&~(0xF<<U1TX*2))|(5<<U1TX*2)); // Clear current settings for U1TX/U1RX, then set to UART1 function

		U1LCR = ((1<<UART_DLAB)|(UART_WRDLEN)); // Set LCR: Enable DLAB (bit 7), set 8-bit word length (bits 0,1 set to 11)
		U1DLL = DVSR;                           // Set Divisor Latch Low (DLL) register for baud rate
		U1DLM = DVSR>>8;                        // Set Divisor Latch High (DLM) register for baud rate
		U1LCR&= ~(1<<UART_DLAB);                // Clear DLAB (bit 7) to disable access to DLL/DLM and enable normal operation
		// Configure P0.0 (U0TX) and P0.1 (U0RX) for UART0 function.
		// (5) directly corresponds to setting P0.0 and P0.1 to Function 01 (UART0 Tx/Rx).
		PINSEL0=((PINSEL0&~0xF)|5); // Clear current settings for U0TX/U0RX, then set to UART0 function

		U0LCR = ((1<<UART_DLAB)|(UART_WRDLEN)); // Set LCR: Enable DLAB (bit 7), set 8-bit word length (bits 0,1 set to 11)
		U0DLL = DVSR;                           // Set Divisor Latch Low (DLL) register for baud rate
		U0DLM = DVSR>>8;                        // Set Divisor Latch High (DLM) register for baud rate
		U0LCR&= ~(1<<UART_DLAB);                // Clear DLAB (bit 7) to disable access to DLL/DLM and enable normal operation
#ifdef UART_INT
		VICIntSelect = 0; 		// IRQ
		VICVectAddr0 = (unsigned)UART0_isr;
		VICVectCntl0 = 0x20 | 6;  	//UART0 Interrupt
		VICIntEnable = 1 << 6;   	// Enable UART0 Interrupt 
		U0IER = 0x01;       		// Enable UART0 RDA Interrupts 
#endif
}

/**
 * @brief Transmits a single byte (word) via the specified UART unit.
 *
 * This function waits until the Transmit Holding Register (THR) is empty
 * (indicated by UART_TEMT bit in LSR) before writing the byte for transmission.
 *
 * @param un The UART unit number (0 for UART0, 1 for UART1).
 * @param word The 8-bit data to be transmitted.
 */
void txUART(u8 un,u8 word){
	if(un){ // If UART1 (un = 1)
		U1THR=word; // Write data to UART1 Transmit Holding Register
		// Wait until the Transmit Empty (TEMT) bit in the Line Status Register (LSR) is set.
		// This indicates that the Transmit Holding Register and Transmit Shift Register are both empty.
		while(((U1LSR>>UART_TEMT)&1)==0);
	}else{ // If UART0 (un = 0)
		U0THR=word; // Write data to UART0 Transmit Holding Register
		// Wait until the Transmit Empty (TEMT) bit in the Line Status Register (LSR) is set.
		while(((U0LSR>>UART_TEMT)&1)==0);
	}
}

/**
 * @brief Receives a single byte from the specified UART unit.
 *
 * This function checks if data is ready in the Receiver Buffer Register (RBR)
 * (indicated by UART_DR bit in LSR) and, if so, reads and returns the byte.
 * It does not block, returning 0 if no data is available.
 *
 * @param un The UART unit number (0 for UART0, 1 for UART1).
 * @return The received 8-bit data, or 0 if no data is ready.
 */
u8 rx_UART(u8 un){
	static u8 word; // Static variable to retain value across calls, though not strictly necessary for this logic
	word=0;         // Initialize word to 0

	if(un){ // If UART1 (un = 1)
		// Check if Data Ready (DR) bit in Line Status Register (LSR) is set.
		// This indicates that valid received data is available in the RBR.
		if((U1LSR>>UART_DR)&1)
			word=U1RBR; // Read data from UART1 Receiver Buffer Register
	}else{ // If UART0 (un = 0)
		// Check if Data Ready (DR) bit in Line Status Register (LSR) is set.
		if((U0LSR>>UART_DR)&1)
			word=U0RBR; // Read data from UART0 Receiver Buffer Register
	}
	return word; // Return the received byte or 0
}

/**
 * @brief Receives a string from the specified UART unit until a newline character is encountered.
 *
 * This function continuously calls `rx_UART` to read characters and stores them
 * in the provided buffer until a newline ('\n') character is received.
 * The newline character itself is typically discarded, and the string is null-terminated.
 * This function assumes UART0 is used based on the current implementation.
 *
 * @param un The UART unit number (0 for UART0, 1 for UART1).
 * @param buf A pointer to the character buffer where the received string will be stored.
 */
void strRx_UART(u8 un,s8* buf){
	u32 i=0; // Index for the buffer
	s8 ch;   // Character to store the received byte

	do{
		// Note: The original code always calls rx_UART(U0) regardless of the 'un' parameter.
		// This might be an oversight if the intention was to use the specified 'un'.
		ch=rx_UART(U0); // Read a byte from UART0
		if(ch)buf[i++]=ch; // If a valid character is received (not 0), store it and increment index
	}while(ch!='\n'); // Continue reading until a newline character is received

	buf[i-1]='\0'; // Null-terminate the string, excluding the newline character
}


/**
 * @brief Transmits a null-terminated string via the specified UART unit.
 *
 * This function iterates through the input string, transmitting each character
 * using `txUART` until the null terminator ('\0') is reached.
 *
 * @param un The UART unit number (0 for UART0, 1 for UART1).
 * @param str A pointer to the null-terminated string to be transmitted.
 */
void strTxUART(u8 un,s8 *str){
	while(*str) // Loop until the null terminator is encountered
		txUART(un,*(str++)); // Transmit the current character and then increment the string pointer
}
