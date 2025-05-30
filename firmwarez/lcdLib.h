#ifndef _LCDLIB_H
#define _LCDLIB_H

#include "types.h" // Include custom type definitions

// Pin definitions for LCD data lines and control lines
// Data lines:
#define LCD_PINS_4B 20 	//@p1.[23-20]:D[7-4] - For 4-bit mode, uses upper nibble of P1
#define LCD_PINS_8B 16 	//@p1.[23-16]:D[7-0] - For 8-bit mode, uses P1.16-P1.23

// Control lines:
#define LCD_RS 	16		//@p0.16 - Register Select (0=Command, 1=Data)
#define LCD_RW 	26 		//@p1.26 - Read/Write (0=Write, 1=Read - not used in this library as only write ops are supported)
#define LCD_EN 	17		//@p0.17 - Enable (falling edge triggered)

// Modes for writeLcd function 'dc' parameter
#define LCD_DATA 1  // Indicates writing data to LCD
#define LCD_CMD  0  // Indicates writing a command to LCD
#define LCD_INT 2   // Special mode for 4-bit initialization sequence (sending 0x20)

// LCD configuration macros
// LCD_MODE determines 4-bit or 8-bit operation
#define LCD_MODE 8  // 8 for 8-bit mode, 4 for 4-bit mode (Default: 8-bit)

// LCD determines the display size, used to set LCD_DIM
#define LCD 32		// Common LCD sizes: 16 (16x1), 32 (16x2), 80 (20x4) (Default: 16x2)


//*****command hex macros*****//

// Function Set Commands (used during initialization)
#define LCD_INIT8 0x30 // Initialize LCD to 8-bit mode
#define LCD_INIT4 0x20 // Initialize LCD to 4-bit mode (actual command is 0x2)

// Basic Control Commands
#define LCD_CLR 0x01      // Clear display and set cursor to home
#define LCD_HOMECUR 0x02  // Set cursor to home position

// Entry Mode Set Commands
#define LCD_SETCURINC 0x06     // Increment cursor after each character (Entry Mode Set: ID=1, SH=0)
#define LCD_SETDISPSHIFT 0x05  // Shift display right after each character (Entry Mode Set: ID=0, SH=1) - rarely used for text entry

// Display ON/OFF Control Commands
#define LCD_DISPON 0x0C            // Display ON, Cursor OFF, Blink OFF
#define LCD_DISPON_BLKON 0x0D      // Display ON, Cursor OFF, Blink ON
#define LCD_DISPON_CURON 0x0E      // Display ON, Cursor ON, Blink OFF
#define LCD_DISPON_CURON_BLKON 0x0F// Display ON, Cursor ON, Blink ON

// Cursor or Display Shift Commands
// Shift cursor/display by 1 character left/right
#define LCD_DISPSHIFT_R 0x1C // Shift entire display right
#define LCD_DISPSHIFT_L 0x18 // Shift entire display left
#define LCD_CURSHIFT_R 	0x14 // Shift cursor right
#define LCD_CURSHIFT_L 	0x10 // Shift cursor left

// Interface (Function Set) commands based on LCD_MODE
#if LCD_MODE==8
#define LCD_16x1 0x30 // Function Set: 8-bit, 1-line, 5x8 dots
#define LCD_16x2 0x3C // Function Set: 8-bit, 2-line, 5x10 dots (or 0x38 for 5x8 dots)
#define LCD_20x4 0x38 // Function Set: 8-bit, 2-line, 5x8 dots (often used for 4-line displays in 2-line mode internally)
#elif LCD_MODE==4
#define LCD_16x1 0x20 // Function Set: 4-bit, 1-line, 5x8 dots
#define LCD_16x2 0x28 // Function Set: 4-bit, 2-line, 5x8 dots
#define LCD_20x4 0x28 // Function Set: 4-bit, 2-line, 5x8 dots
#endif

// Determine LCD dimensions based on LCD macro
#if LCD==16
	#define LCD_DIM LCD_16x1      // 16 characters x 1 line
	#define LCD_MAX_ROWS 1        // Maximum number of rows
	#define LCD_MAX_COLS 16       // Maximum number of columns
#elif LCD==32
	#define LCD_DIM LCD_16x2      // 16 characters x 2 lines
	#define LCD_MAX_ROWS 2
	#define LCD_MAX_COLS 16
#elif LCD==80
	#define LCD_DIM LCD_20x4      // 20 characters x 4 lines
	#define LCD_MAX_ROWS 4
	#define LCD_MAX_COLS 20
#endif


// Set DDRAM (Display Data RAM) address (cursor position)
#define LCD_CGRAM 0x40 // Set CGRAM address (for custom characters)
// Set DDRAM address for specific rows
#define LCD_ROW0 0x80//0x80|0x00 - Address of the first character of row 0
#define LCD_ROW1 0xC0//0x80|0x40 - Address of the first character of row 1
#define LCD_ROW2 0x94//0x80|0x14 - Address of the first character of row 2 (for 20x4 LCDs)
#define LCD_ROW3 0xD4//0x80|0x54 - Address of the first character of row 3 (for 20x4 LCDs)
//***

// Custom character pixel data (externly defined in lcdLib.c)
extern const s8 bell[];       // Pixel data for a bell icon
extern const s8 heartNull[];  // Pixel data for an empty heart icon
extern const s8 heartFull[];  // Pixel data for a full heart icon

//*****FUNCTION DECLARATIONS*****
	
/**
 * @brief Initializes the LCD module.
 * Configures GPIO pins for LCD control, performs the LCD initialization sequence
 * for either 4-bit or 8-bit mode, clears the display, and sets entry mode.
 */
void initLcd(void);

/**
 * @brief Writes a command or data byte to the LCD.
 * Handles the communication protocol for both 4-bit and 8-bit modes.
 * @param word The byte to write (command or data).
 * @param dc   Control signal: LCD_DATA (1) for data, LCD_CMD (0) for command, LCD_INT (2) for 4-bit init.
 */
void writeLcd(s8 word,s16 dc);

/**
 * @brief Moves the LCD cursor to a specified row and column.
 * @param row The target row (0 to LCD_MAX_ROWS-1).
 * @param col The target column (0 to LCD_MAX_COLS-1).
 */
void moveLcdCursor(s16 row,s16 col);

/**
 * @brief Creates a new custom character in CGRAM.
 * @param pixels An array of 8 bytes, where each byte represents a row of pixels for the character.
 * @param asc The ASCII value (0-7) where the custom character will be stored in CGRAM.
 */
void makeNewCharAt(const s8 *pixels,s16 asc);

/**
 * @brief Displays a single character on the LCD at the current cursor position.
 * @param ch The character to display.
 */
void char2Lcd(s8 ch);

/**
 * @brief Displays a null-terminated string on the LCD at the current cursor position.
 * @param str The string to display.
 */
void str2Lcd(const s8 *str);

/**
 * @brief Displays a signed 32-bit integer on the LCD.
 * @param inum The integer to display.
 */
void int2Lcd(s32 inum);

/**
 * @brief Displays an unsigned 32-bit integer on the LCD.
 * @param inum The unsigned integer to display.
 */
void uInt2Lcd(u32 inum);

/**
 * @brief Displays a floating-point number on the LCD (up to 2 decimal places).
 * @param fnum The float to display.
 */
void flt2Lcd(f32 fnum);

/**
 * @brief Placeholder function for displaying a float as a string (not implemented).
 * @param f32 The float to display.
 */
void f32StrLcd(f32);

/**
 * @brief Displays a null-terminated string on the LCD at a specified row and column.
 * @param str The string to display.
 * @param row The target row.
 * @param col The target column.
 */
void str2LcdPos(s8 *str,s16 row,s16 col);

/**
 * @brief Displays a single character on the LCD at a specified row and column.
 * @param ch The character to display.
 * @param row The target row.
 * @param col The target column.
 */
void char2LcdPos(s8 ch,s16 row,s16 col);

/**
 * @brief Clears the entire LCD display and sets the cursor to home.
 */
void clearLcdDisplay(void);

/**
 * @brief Clears a specific row on the LCD.
 * @param row The row to clear.
 */
void clearLcdRow(s16 row);

// Special display functions
/**
 * @brief Displays an unsigned 32-bit number in binary format on the LCD.
 * @param n The number to display.
 * @param bits The number of bits to display (e.g., 7 for an 8-bit number excluding MSB).
 */
void bin2Lcd(u32 n,s16 bits);

/**
 * @brief Displays an unsigned 32-bit number in hexadecimal format on the LCD.
 * @param n The number to display.
 */
void hex2Lcd(u32 n);

/**
 * @brief Displays an unsigned 32-bit number in octal format on the LCD.
 * @param n The number to display.
 */
void oct2Lcd(u32 n);

/**
 * @brief Scrolls a string horizontally on a specified LCD row.
 * This function creates an infinite horizontal scrolling effect.
 * @param str The string to scroll.
 * @param row The row on which to scroll the string.
 */
void scrllStr2Lcd(s8 *str,s16 row);

#endif
