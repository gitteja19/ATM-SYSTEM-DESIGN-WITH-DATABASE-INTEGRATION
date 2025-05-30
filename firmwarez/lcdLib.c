#include "types.h"   // Include custom type definitions
#include "delayLib.h"// Include delay library for timing
#include "gpioLib.h" // Include GPIO library for pin control
#include "lcdLib.h"  // Include LCD library header

// Custom character pixel data definitions
// Each array of 8 bytes represents an 8x5 pixel custom character.
// MSB (bit 7) is typically ignored, bits 4-0 represent the 5 pixel columns.
const s8 bell[]		={0x04,0x0E,0x0E,0x0E,0x1F,0x00,0x04,0x00};     // Bell icon pixels
const s8 lockIcon[]  ={0x00,0x0E,0x11,0x1F,0x15,0x1F,0x1F,0x00};   // Lock icon pixels
const s8 unlockIcon[]={0x0E,0x11,0x01,0x1F,0x15,0x1F,0x1F,0x00}; // Unlock icon pixels



/////GENERAL LCD FUNCTIONS

/**
 * @brief Initializes the LCD module.
 *
 * This function configures the necessary GPIO pins for LCD control (RS, EN, Data),
 * performs the LCD initialization sequence based on 4-bit or 8-bit mode,
 * clears the display, and sets the entry mode.
 */
void initLcd(){
	// GPIO pin configuration
	// LCD RS (Register Select) pin as output
	INITPIN(GPIO0,LCD_RS,1);
	// LCD RW (Read/Write) pin as output (commented out, implying write-only operation)
	//INITPIN(GPIO1,LCD_RW,1);
	// LCD EN (Enable) pin as output
	INITPIN(GPIO0,LCD_EN,1);

#if LCD_MODE==4 // If 4-bit LCD mode is selected
	// Initialize 4 data pins (D4-D7) on GPIO1 as outputs
	INIT4PINS(GPIO1,LCD_PINS_4B,0xF);
#elif LCD_MODE==8 // If 8-bit LCD mode is selected
	// Initialize 8 data pins (D0-D7) on GPIO1 as outputs
	INIT8PINS(GPIO1,LCD_PINS_8B,0xFF);
#endif

	//// Start LCD initialization sequence ////
	delayMs(15); // Wait for more than 15ms after power-on

	// Send 0x30 command (8-bit interface) three times for robust initialization.
	// This command is recognized by both 4-bit and 8-bit LCDs to switch to 8-bit mode.
	writeLcd(LCD_INIT8,LCD_CMD);
	delayMs(5);
	writeLcd(LCD_INIT8,LCD_CMD);
	delayUs(200);
	writeLcd(LCD_INIT8,LCD_CMD);
	delayUs(200);

#if LCD_MODE==4 // Specific initialization for 4-bit mode
	// Send 0x20 command (Function Set with 4-bit interface).
	// LCD_INT mode in writeLcd sends only the upper nibble (0x2).
	writeLcd(LCD_INIT4,LCD_INT);
#endif

	// Set interface length, number of display lines, and character font.
	// LCD_DIM macro ensures correct command based on LCD_MODE and LCD type.
	writeLcd(LCD_DIM,LCD_CMD);
	// Display ON/OFF Control: Display ON, Cursor OFF, Blink OFF.
	writeLcd(LCD_DISPON,LCD_CMD);
	// Clear Display: Clears all display data and sets cursor to home.
	writeLcd(LCD_CLR,LCD_CMD);
	// Entry Mode Set: Set cursor increment mode after each character.
	writeLcd(LCD_SETCURINC,LCD_CMD);
	delayMs(2); // Small delay after clear display.
}

/**
 * @brief Writes a byte (command or data) to the LCD.
 *
 * This function handles the communication protocol with the LCD, including
 * setting RS line, writing data/command, and pulsing the EN line.
 * It supports both 4-bit and 8-bit LCD modes.
 *
 * @param word The byte of data or command to be sent to the LCD.
 * @param dc   Control signal:
 * - LCD_DATA (1): Write 'word' as displayable data.
 * - LCD_CMD (0): Write 'word' as a command.
 * - LCD_INT (2): Special case for 4-bit initialization (sends only upper nibble).
 */
void writeLcd(s8 word,s16 dc){
	// Set RS (Register Select) pin based on whether it's data or command
	if(dc==1)SETPIN(GPIO0,LCD_RS);// dc=1 -> DATA (RS high)
	else if(dc==0) CLRPIN(GPIO0,LCD_RS);// dc=0 -> COMMAND (RS low)
	else if(dc==2){// dc=2 -> Special initialization for 4-bit mode (sends 0x20)
		CLRPIN(GPIO0,LCD_RS); // RS low for command
		// Write only the upper nibble (0x2) of the command (0x20)
		WRITE4PINS(GPIO1,LCD_PINS_4B,(word >> 4));
		// Pulse the Enable (EN) pin to latch the data/command
		SETPIN(GPIO0,LCD_EN);
		delayUs(1); // Short delay for EN pulse width
		CLRPIN(GPIO0,LCD_EN);
		return; // Exit as only upper nibble is sent for LCD_INT mode
	}

#if LCD_MODE==8 // 8-bit LCD mode
	// Write the full 8-bit 'word' to the data pins
	WRITE8PINS(GPIO1,LCD_PINS_8B,word);
	// Pulse the Enable (EN) pin to latch the data/command
	SETPIN(GPIO0,LCD_EN);
	delayUs(1); // Short delay for EN pulse width
	CLRPIN(GPIO0,LCD_EN);
#elif LCD_MODE==4 // 4-bit LCD mode (data is sent in two 4-bit nibbles)
	// Write the upper nibble (D7-D4) of the 'word' first
	WRITE4PINS(GPIO1,LCD_PINS_4B,(word >> 4));
	// Pulse EN to latch the upper nibble
	SETPIN(GPIO0,LCD_EN);
	delayUs(1);
	CLRPIN(GPIO0,LCD_EN);
	// Write the lower nibble (D3-D0) of the 'word' next
	WRITE4PINS(GPIO1,LCD_PINS_4B,(word & 0x0F));
	// Pulse EN to latch the lower nibble
	SETPIN(GPIO0,LCD_EN);
	delayUs(1);
	CLRPIN(GPIO0,LCD_EN);
#endif
	// Wait for LCD to process the command/data.
	// This delay is crucial for the LCD to complete internal operations.
	delayUs(60); // A typical delay (e.g., 37us for most commands, 1.52ms for clear/home)
}

/**
 * @brief Creates a new custom character in the LCD's Character Generator RAM (CGRAM).
 *
 * A custom character is defined by an 8-byte array, where each byte represents a row of pixels.
 * The character can then be displayed by writing its ASCII value (0-7).
 *
 * @param pixels An array of 8 bytes, where each byte defines one row of the character (LSB is rightmost pixel).
 * @param asc The ASCII code (0-7) where the custom character will be stored in CGRAM.
 * The LCD supports up to 8 custom characters.
 */
void makeNewCharAt(const s8 *pixels,s16 asc){
	u16 i;
	if(asc>8)return;// Invalid ASCII code for custom character (only 0-7 allowed)

	// Set CGRAM address: LCD_CGRAM (0x40) + (ASCII code * 8 bytes per character)
	writeLcd(LCD_CGRAM+asc*8,LCD_CMD);
	// Write each of the 8 pixel rows to CGRAM
	for(i=0;i<8;i++)char2Lcd(pixels[i]);
}

/**
 * @brief Displays a single character on the LCD at the current cursor position.
 * @param ch The ASCII value of the character to display.
 */
void char2Lcd(s8 ch){
	writeLcd(ch,LCD_DATA); // Send the character as data to the LCD
}

/**
 * @brief Displays a null-terminated string on the LCD, starting from the current cursor position.
 * @param str A pointer to the null-terminated string to display.
 */
void str2Lcd(const s8 *str){
	while(*str){ // Loop until the null terminator is encountered
		writeLcd(*(str++),LCD_DATA); // Send each character as data and increment pointer
	}
}

/**
 * @brief Moves the LCD cursor to a specified row and column.
 *
 * This function calculates the appropriate DDRAM address based on the desired
 * row and column and sends the corresponding command to the LCD.
 *
 * @param row The target row (0-indexed, e.g., 0 for first row, 1 for second).
 * @param col The target column (0-indexed, e.g., 0 for first column).
 */
void moveLcdCursor(s16 row,s16 col){
	// Calculate and send the command to set the DDRAM address.
	// The MSB (D7) is always set for DDRAM address commands (0x80).
	// Row addresses vary: 0x80 for row 0, 0xC0 for row 1, 0x94 for row 2, 0xD4 for row 3 (for 20x4 LCDs).
	if(row==0)writeLcd(LCD_ROW0+col,LCD_CMD);		// Go to row 0, address 0x80 + col
	else if(row==1)writeLcd(LCD_ROW1+col,LCD_CMD);	// Go to row 1, address 0xC0 + col
	else if(row==2)writeLcd(LCD_ROW2+col,LCD_CMD);	// Go to row 2, address 0x94 + col
	else if(row==3)writeLcd(LCD_ROW3+col,LCD_CMD);	// Go to row 3, address 0xD4 + col
}

/**
 * @brief Displays a null-terminated string on the LCD at a specified row and column.
 * This function first moves the cursor and then prints the string.
 *
 * @param str A pointer to the null-terminated string to display.
 * @param row The target row.
 * @param col The target column.
 */
void str2LcdPos(s8 *str,s16 row,s16 col){
	// Move the cursor to the desired position
	if(row==0)writeLcd((LCD_ROW0+col),LCD_CMD);		// Go to row 0 + col
	else if(row==1)writeLcd((LCD_ROW1+col),LCD_CMD);	// Go to row 1 + col

	while(*str){ // Loop until the null terminator
		writeLcd(*(str++),LCD_DATA);	// Send each character as data and increment pointer
	}
}

/**
 * @brief Displays a single character on the LCD at a specified row and column.
 * This function first moves the cursor and then prints the character.
 *
 * @param ch The character to display.
 * @param row The target row.
 * @param col The target column.
 */
void char2LcdPos(s8 ch,s16 row,s16 col){
	// Move the cursor to the desired position
	if(row==0) writeLcd((LCD_ROW0+col),LCD_CMD);		// Go to row 0 + col
	else if(row==1) writeLcd((LCD_ROW1+col),LCD_CMD);	// Go to row 1 + col

	writeLcd(ch,LCD_DATA); // Send the character as data
}

/**
 * @brief Clears the entire LCD display and sets the cursor to the home position (0,0).
 */
void clearLcdDisplay(){
    writeLcd(0x01,LCD_CMD);  // Send Clear Display command (0x01)
    delayMs(2); // Requires a longer delay than other commands (typically ~1.52ms)
}

/**
 * @brief Clears a specific row on the LCD by writing spaces across it.
 *
 * @param row The row to clear (0-indexed).
 */
void clearLcdRow(s16 row){
	u8 col;
	// Set cursor to the beginning of the specified row
	if(row==0)writeLcd(LCD_ROW0,LCD_CMD);		// Go to row 0
	else if(row==1)writeLcd(LCD_ROW1,LCD_CMD);	// Go to row 1
	else if(row==2)writeLcd(LCD_ROW2,LCD_CMD);	// Go to row 2
	else if(row==3)writeLcd(LCD_ROW3,LCD_CMD);	// Go to row 3

	// Write spaces to clear the entire row
	for(col=0;col<LCD_MAX_COLS;col++){  // Iterate through all columns in the row
		writeLcd(' ',LCD_DATA); // Write a space character
    }
	// Reset cursor to the beginning of the cleared row (optional, but good practice)
	if(row==0)writeLcd(LCD_ROW0,LCD_CMD);
	else if(row==1)writeLcd(LCD_ROW1,LCD_CMD);
	else if(row==2)writeLcd(LCD_ROW2,LCD_CMD);
	else if(row==3)writeLcd(LCD_ROW3,LCD_CMD);
}

/**
 * @brief Displays a signed 32-bit integer on the LCD.
 * Converts the integer to a string and then prints it. Handles negative numbers.
 *
 * @param inum The signed integer to display.
 */
void int2Lcd(s32 inum){
	s8 str[11]; // Buffer to hold the string representation (max 10 digits + sign + null terminator)
	s16 sign,i;
	sign=0,i=0;

	if(inum<0){ // Check if the number is negative
		sign=1; // Set sign flag
		inum*=-1; // Convert to positive for digit extraction
	}

	// Extract digits in reverse order
	do{
		str[i++]='0'+(inum%10); // Convert last digit to ASCII character and store
		inum/=10; // Remove last digit
	}while(inum); // Continue until all digits are extracted

	if(sign)str[i++]='-'; // Add negative sign if applicable

	// Print the string in correct order (from end to beginning of buffer)
	for(--i;i>=0;i--){
		writeLcd(str[i],LCD_DATA);
	}
}

/**
 * @brief Displays an unsigned 32-bit integer on the LCD.
 * Converts the unsigned integer to a string and then prints it.
 *
 * @param inum The unsigned integer to display.
 */
void uInt2Lcd(u32 inum){
	s8 str[10]; // Buffer to hold the string representation (max 10 digits + null terminator)
	s16 i;
	i=0;

	// Extract digits in reverse order
	do{
		str[i++]='0'+(inum%10); // Convert last digit to ASCII character and store
		inum/=10; // Remove last digit
	}while(inum); // Continue until all digits are extracted

	// Print the string in correct order (from end to beginning of buffer)
	for(--i;i>=0;i--){
		writeLcd(str[i],LCD_DATA);
	}
}

/**
 * @brief Displays a floating-point number on the LCD.
 * Displays the integer part, a decimal point, and then two decimal places.
 * Handles negative floating-point numbers.
 *
 * @param fnum The float number to display.
 */
void flt2Lcd(f32 fnum){
	u32 iNum,i; // Integer part and loop counter

	if(fnum<0){ // Check if the number is negative
		char2Lcd('-'); // Display negative sign
		fnum=-fnum; // Convert to positive for processing
	}

	iNum=(u32)fnum; // Extract the integer part
	fnum-=iNum;     // Get the fractional part

	uInt2Lcd(iNum); // Display the integer part
	char2Lcd('.');  // Display the decimal point

	// Display two decimal places
	for(i=0;i<2;i++){
		fnum*=10;       // Shift one decimal place left
		iNum= (u32)fnum; // Get the digit after the decimal point
		char2Lcd(iNum+'0'); // Display the digit
		fnum-=iNum;     // Remove the displayed digit from the fractional part
	}
}

/**
 * @brief Placeholder function for displaying a float as a string.
 * (Not implemented in the provided code)
 * @param fnum The float number to display.
 */
void f32StrLcd(f32 fnum){
	// Implementation would involve converting float to string using sprintf or similar logic
	// and then printing the string to LCD.
}

/**
 * @brief Displays an unsigned 32-bit number in binary format on the LCD.
 *
 * @param num The unsigned integer to display.
 * @param bits The number of bits to display (e.g., 7 for an 8-bit number excluding MSB, or 31 for full 32-bit).
 */
void bin2Lcd(u32 num,s16 bits){
	s16 i;
	// Loop from the most significant bit to the least significant bit
	for(i=bits;i>=0;i--){
		// Extract the bit at position 'i' and convert to '0' or '1' character
		writeLcd(('0'+((num>>i)&1)),LCD_DATA);
	}
}

/**
 * @brief Displays an unsigned 32-bit number in hexadecimal format on the LCD.
 * Converts the number to hexadecimal string and then prints it.
 *
 * @param num The unsigned integer to display.
 */
void hex2Lcd(u32 num){
	s8 str[8]; // Buffer to hold the hexadecimal string (max 8 hex digits for 32-bit)
	s16 i,t;
	i=0;

	// Extract hexadecimal digits in reverse order
	do{
		t=(num%16); // Get the last hexadecimal digit
		if(t<10)str[i++]='0'+t; // Convert 0-9 to '0'-'9'
		else str[i++]='A'+t-10; // Convert 10-15 to 'A'-'F'
		num/=16; // Remove the last hexadecimal digit
	}while(num); // Continue until all digits are extracted

	// Print the string in correct order (from end to beginning of buffer)
	for(--i;i>=0;i--){
		writeLcd(str[i],LCD_DATA);
	}
}

/**
 * @brief Displays an unsigned 32-bit number in octal format on the LCD.
 * Converts the number to octal string and then prints it.
 *
 * @param num The unsigned integer to display.
 */
void oct2Lcd(u32 num){
	s8 str[8]; // Buffer to hold the octal string (max 11 digits for 32-bit, but 8 is usually enough for common uses)
	s16 i;
	i=0;

	// Extract octal digits in reverse order
	do{
		str[i++]='0'+(num%8); // Get the last octal digit and convert to ASCII
		num/=8; // Remove the last octal digit
	}while(num); // Continue until all digits are extracted

	// Print the string in correct order (from end to beginning of buffer)
	for(--i;i>=0;i--){
		writeLcd(str[i],LCD_DATA);
	}
}

/**
 * @brief Scrolls a string horizontally on a specified LCD row.
 * This function creates an infinite horizontal scrolling effect by repeatedly
 * displaying a segment of the string and shifting it.
 *
 * @param str A pointer to the null-terminated string to scroll.
 * @param row The row on which to scroll the string.
 */
void scrllStr2Lcd(s8*str,s16 row){
	s32 len=0,i=0,j,k,l,m;
	s8 *temp=str;

	// Calculate string length
	while(*temp){
		len++;
		temp++;
	}

	// Scrolling loop (infinite loop, typically stopped by external logic)
	while(1){
		// 'j' calculates the starting column for the current segment, creating a scrolling effect.
		// It cycles from (LCD_MAX_COLS-1) down to 0, then repeats.
		j= LCD_MAX_COLS-1-((i++)%LCD_MAX_COLS);

		// Determine 'm' which controls how many times the string segments are displayed
		// within a full "scroll cycle" for the current 'j' position.
		if(j){
			// When 'j' is not 0 (i.e., cursor is not at the very left edge),
			// only one segment is displayed, shifting from right to left.
			m=1;
		}else if(j==0){
			// When 'j' is 0 (i.e., cursor is at the very left edge),
			// the entire string will be scrolled character by character if it's longer than a row.
			m=len;
		}

		// Inner loop to display parts of the string for the current scroll position 'j'
		for(l=0;l<m;l++){
				moveLcdCursor(row,j); // Move cursor to the starting position for the current segment
				// Display a segment of the string from (str + l) up to LCD_MAX_COLS characters
				for(k=0;k<LCD_MAX_COLS-j;k++){
					if(k+l>=len)break; // Stop if end of string is reached
					char2Lcd(str[k+l]); // Display character
				}
				delayMs(60); // Small delay to control scrolling speed
				clearLcdRow(row); // Clear the row after displaying the segment to prepare for the next frame
			}
	}
}
