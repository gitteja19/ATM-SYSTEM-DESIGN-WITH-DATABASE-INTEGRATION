#include <string.h> // Include string manipulation functions (strlen)
#include "types.h"  // Include custom type definitions
#include "uartLib.h"// Include UART library for communication with RFID reader

// Assuming uart1 is already initialized for RFID communication.
// The RFID reader is expected to be connected to UART1.

/**
 * @brief Reads data from the RFID reader into a buffer.
 *
 * This function continuously reads bytes from UART1 until an "End of Text" (0x03)
 * character is received, which typically marks the end of an RFID tag's data.
 * The received bytes are stored in the provided buffer.
 *
 * @param buf A pointer to the character buffer where the RFID data will be stored.
 */
void getRFID(s8 *buf){
	u32 i=0; // Index for the buffer
	s8 ch;   // Character to store the received byte

	do{
		ch=rx_UART(U1); // Read a byte from UART1
		if(ch)buf[i++]=ch; // If a valid character is received (not 0), store it and increment index
	}while(ch!=0x03); // Continue reading until 0x03 (End of Text) character is received

	buf[i]='\0'; // Null-terminate the string to make it a valid C string
}

/**
 * @brief Validates the format of the received RFID data.
 *
 * This function checks if the RFID data in the buffer conforms to an expected format:
 * - The length of the string must be 10 characters.
 * - The first character must be a "Start of Text" (0x02) character.
 * - The last character must be an "End of Text" (0x03) character.
 *
 * @param buf A pointer to the character buffer containing the RFID data.
 * @return 1 if the RFID data format is valid, 0 otherwise.
 */
s32 isRfidOk(s8 *buf){
 	if(strlen(buf)!=10)return 0; // Check if the string length is exactly 10
	if((buf[0]!=0x02)||(buf[9]!=0x03))return 0; // Check for Start of Text (0x02) at buf[0] and End of Text (0x03) at buf[9]
	return 1; // If all checks pass, the RFID data is considered OK
}
