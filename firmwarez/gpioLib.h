#ifndef _GPIOLIB_H
#define _GPIOLIB_H

#include "types.h" // Include custom type definitions

// Direction macros for GPIO pins
#define OUT 1 // Output direction
#define IN  0 // Input direction

// Base addresses for GPIO Port 0 and Port 1 registers
// These are memory-mapped addresses for accessing GPIO control registers (e.g., IOSET, IOCLR, IODIR, IOPIN)
#define GPIO0 ((vu32 *)0xE0028000) // Base address for GPIO Port 0
#define GPIO1 ((vu32 *)0xE0028010) // Base address for GPIO Port 1

// Macro to initialize the direction of a single GPIO pin.
// (portAdd+2) typically points to the IODIR register.
// It clears the bit corresponding to the pin and then sets it according to the 'dir' (OUT/IN).
#define INITPIN(portAdd,pin,dir)		(*(portAdd+2)=(((*(portAdd+2))&~(1<<pin))|(dir<<pin)))

// Macro to set a single GPIO pin high.
// (portAdd+1) typically points to the IOSET register.
#define SETPIN(portAdd,pin)			 	(*(portAdd+1)= (1<<pin))

// Macro to clear a single GPIO pin low.
// (portAdd+3) typically points to the IOCLR register.
#define CLRPIN(portAdd,pin)		  	 	(*(portAdd+3)= (1<<pin))

// Macro to read the current state of a single GPIO pin.
// (*portAdd) typically points to the IOPIN register.
#define READPIN(portAdd,pin)	     	(((*portAdd)>>pin)&1)

// Macro to toggle the state of a single GPIO pin.
// This directly manipulates the IOPIN register using XOR.
#define TGLPIN(portAdd,pin)    			((*portAdd)	^=(1<<pin))

// Macro to write a specific value (HIGH/LOW) to a single GPIO pin.
// This directly manipulates the IOPIN register.
#define WRITEPIN(portAdd,pin,val)    	(*portAdd=(((*portAdd)&~(1<<pin))|(val<<pin)))

// Macro to initialize the direction of 4 consecutive GPIO pins starting from 'pin'.
// 'dir' is a 4-bit value where each bit sets the direction for a pin (0x0F for all outputs, 0x00 for all inputs).
#define INIT4PINS(portAdd,pin,dir)		(*(portAdd+2)=(((*(portAdd+2))&~(0xF<<pin))|(dir<<pin)))

// Macro to read the state of 4 consecutive GPIO pins starting from 'pin'.
// The result is a 4-bit value.
#define READ4PINS(portAdd,pin)	     	(((*portAdd)>>pin)&0xF)

// Macro to write a 4-bit 'data' value to 4 consecutive GPIO pins starting from 'lpin' (least significant pin).
#define WRITE4PINS(portAdd,lpin,data) 	((*portAdd)=(((*portAdd)&~(0xF<<lpin))|(data<<lpin)))

// Macro to initialize the direction of 8 consecutive GPIO pins starting from 'pin'.
// 'dirVal' is an 8-bit value (0xFF for all outputs, 0x00 for all inputs).
#define INIT8PINS(portAdd,pin,dirVal) 	(*(portAdd+2)=(((*(portAdd+2))&~(0xFF<<pin))|(dirVal<<pin)))

// Macro to set 8 consecutive GPIO pins high starting from 'lpin'.
#define SET8PINS(portAdd,lpin)			(*(portAdd+1)= (0xFF<<lpin))

// Macro to clear 8 consecutive GPIO pins low starting from 'lpin'.
#define CLR8PINS(portAdd,lpin)			(*(portAdd+3)= (0xFF<<lpin))

// Macro to read the state of 8 consecutive GPIO pins starting from 'pin'.
// The result is an 8-bit value.
#define READ8PINS(portAdd,pin)	     	(((*portAdd)>>pin)&0xFF)

// Macro to write an 8-bit 'data' value to 8 consecutive GPIO pins starting from 'lpin'.
#define WRITE8PINS(portAdd,lpin,data) 	(*portAdd=(((*portAdd)&~(0xFF<<lpin))|(data<<lpin)))


#endif
