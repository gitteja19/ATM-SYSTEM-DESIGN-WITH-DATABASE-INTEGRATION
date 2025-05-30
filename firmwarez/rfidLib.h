#ifndef _RFIDLIB_H
#define _RFIDLIB_H

#include "types.h" // Include custom type definitions

/**
 * @brief Reads data from the RFID reader into a buffer.
 *
 * This function continuously reads bytes from UART1 until an "End of Text" (0x03)
 * character is received, which typically marks the end of an RFID tag's data.
 * The received bytes are stored in the provided buffer.
 *
 * @param buf A pointer to the character buffer where the RFID data will be stored.
 */
void getRFID(s8 *buf);

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
s32 isRfidOk(s8 *buf);


#endif
