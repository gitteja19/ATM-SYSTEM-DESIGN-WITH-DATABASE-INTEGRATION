// keypad matrix -definition.c file

#include <lpc214x.h> // Include LPC214x specific header (for SFRs if needed, though not directly used in this snippet)
#include "types.h"   // Custom type definitions
#include "gpioLib.h" // GPIO library for pin control
#include "kpmLib.h"  // Keypad library header
#include "delayLib.h"// Delay library for debouncing

// ATM keypad layout example
// This 2D array maps the physical (row, column) position on the keypad
// to the corresponding character or value.
s32 label2D[4][4]=
{
	{'1' ,'2','3' ,'A'},// Row 0: 1, 2, 3, A
	{'4' ,'5','6' ,'B'},// Row 1: 4, 5, 6, B
	{'7' ,'7','9' ,'C'},// Row 2: 7, 8, 9, C (Note: '7' is duplicated, probably a typo, should be '8')
	{'*' ,'0','#' ,'D'} // Row 3: *, 0, #, D
	// * ->'\b' (Backspace/Clear)
	// # ->'\n' (Enter/Confirm)
	// A ->up (Up arrow/Function A)
	// B ->dn (Down arrow/Function B)
};

// Functions

/**
 * @brief Initializes the 4x4 keypad matrix.
 *
 * Configures the keypad row pins as outputs and column pins as inputs.
 * This setup allows scanning the rows by driving them low one by one and
 * checking which column lines go low in response.
 */
void initKpm4x4(void){
	// Initialize the 4 row pins (KPM_R0 to KPM_R3) on GPIO1 as outputs.
	// 0xF (binary 1111) sets all 4 pins in the specified range as outputs.
	INIT4PINS(GPIO1,KPM_R0,0xF);
	// Initialize the 4 column pins (KPM_C0 to KPM_C3) on GPIO1 as inputs.
	// 0x0 (binary 0000) sets all 4 pins in the specified range as inputs.
	INIT4PINS(GPIO1,KPM_C0,0x0);
}

/**
 * @brief Reads a single key press from the 4x4 keypad.
 *
 * This function iterates through each row, drives it low, and then reads
 * the state of the columns. If a column goes low, a key press is detected.
 * It includes a basic debouncing mechanism by waiting for the key release.
 *
 * @return The ASCII value of the pressed key, or 0 if no key is pressed.
 */
s32 readKpm4x4(void ){
	s16 i,j,stat=0; // Loop counters and status variable

	// Iterate through each row (0 to ROWS-1)
	for(i=0;i<ROWS;i++){
		// Send a low signal to the current row while keeping others high.
		// ((1<<i)^0xF) creates a mask where only the 'i'-th bit is 0, and others are 1.
		// For example, if i=0, (1<<0)^0xF = 0x1^0xF = 0xE (1110) - drives R0 low.
		// If i=1, (1<<1)^0xF = 0x2^0xF = 0xD (1101) - drives R1 low.
		WRITE4PINS(GPIO1,KPM_R0,((1<<i)^0xF));

		// Read the state of the column pins.
		// Columns are assumed to be pulled high internally or externally, so a key press
		// will pull the corresponding column low.
		stat=READ4PINS(GPIO1,KPM_C0);// default high (0xF if no key pressed)

		// Check if any key is pressed in the current row.
		// If stat is not 0xF, it means at least one column pin is low.
		if(stat!=0xF){
			// Key press found.
			// Wait until the key is released (debouncing).
			// This prevents multiple detections for a single press.
			while(READ4PINS(GPIO1,KPM_C0)!=0xF);

			// Check which specific column (and thus which key) was pressed.
			for(j=0;j<COLS;j++){
				// If the j-th bit of 'stat' is 0, it means the j-th column is low.
				if(!((stat>>j)&1)){
					delayMs(200); // Small delay to ensure stable reading after release (additional debouncing)
					return label2D[i][j]; // Return the character mapped to the pressed key
				}
			}
		}
	}
	return 0; // No key pressed
}
