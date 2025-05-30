// keypad matrix -header.h file
#ifndef _KPMLIB_H
#define _KPMLIB_H

#include "types.h" // Include custom type definitions

// Macros for keypad dimensions
#define ROWS 4 // Number of rows in the keypad matrix (typically outputs)
#define COLS 4 // Number of columns in the keypad matrix (typically inputs)


// Pin definitions for 4x4 keypad matrix on LPC214x microcontroller (Port 1)
// Rows (output pins)
#define KPM_R0 24 //@P1.[24] - Row 0
#define KPM_R1 25 // @P1.[25] - Row 1
#define KPM_R2 26 // @P1.[26] - Row 2
#define KPM_R3 27 // @P1.[27] - Row 3
// Columns (input pins)
#define KPM_C0 28 // @P1.[28] - Column 0
#define KPM_C1 29 // @P1.[29] - Column 1
#define KPM_C2 30 // @P1.[30] - Column 2
#define KPM_C3 31 // @P1.[31] - Column 3

/*
// Alternative pin definitions for 4x4 keypad matrix on LPC214x microcontroller (Port 0)
// Uncomment and adjust if using Port 0
#define KPM_R0 4 //@P0.[4]
#define KPM_R1 5 //@P0.[5]
#define KPM_R2 6 //@P0.[6]
#define KPM_R3 7 //@P0.[7]

#define KPM_C0 8 //@P0.[8]
#define KPM_C1 9 //@P0.[9]
#define KPM_C2 10 //@P0.[10]
#define KPM_C3 11 //@P0.[11]
*/

// Global arrays for keypad labels
extern s32 label2D[ROWS][COLS];       // 2D array for 4x4 keypad character mapping
extern s32 label3D[3][COLS][COLS];    // Placeholder/future use for a 3D keypad mapping (e.g., shift levels)

// Function declarations for keypad operations
/**
 * @brief Initializes the 4x4 keypad matrix.
 * Configures the row pins as outputs and column pins as inputs.
 */
void initKpm4x4(void);

/**
 * @brief Reads a single key press from the 4x4 keypad.
 * Scans the keypad matrix and returns the ASCII value of the pressed key.
 * Implements basic debouncing.
 * @return The ASCII value of the pressed key, or 0 if no key is pressed.
 */
s32 readKpm4x4(void);

/**
 * @brief Reads a key press from a 4x4x4 (3-layer) keypad.
 * @return The ASCII value of the pressed key.
 */
s32 readKpm4x4x4(void);


#endif
