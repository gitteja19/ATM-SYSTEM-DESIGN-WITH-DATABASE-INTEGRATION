
//types.h

#ifndef _TYPES_H
#define _TYPES_H


// Aliasing standard data types for specific bit lengths and signedness
typedef unsigned char u8;   // Unsigned 8-bit integer
typedef char s8;           // Signed 8-bit integer
typedef unsigned short u16; // Unsigned 16-bit integer
typedef short s16;          // Signed 16-bit integer
typedef unsigned int u32;   // Unsigned 32-bit integer
typedef int s32;            // Signed 32-bit integer
typedef float f32;          // 32-bit floating-point number

typedef	volatile u32 vu32;  // Volatile unsigned 32-bit integer, used for memory-mapped registers

// Common boolean/state macros
#define HIGH 1  // Represents a high logic level or active state
#define LOW  0  // Represents a low logic level or inactive state

#endif
