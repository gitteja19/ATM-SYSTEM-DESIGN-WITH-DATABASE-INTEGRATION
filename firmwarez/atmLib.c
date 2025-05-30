//atmLib.c - Implementation file for ATM functionalities
#include <string.h>    // Include string manipulation functions like strcmp, strncpy, strlen
#include "atmLib.h"    // Include the header file for ATM library functions

// Macro to check if a character is a digit
#define ISNUM(ch) ((ch>='0')&&(ch<='9'))

// Volatile global buffer for UART communication (Tx/Rx messages)
volatile s8 buf[BUF_MAX]="",dummy;
// Volatile global flags and variables for UART receive status, session time, and buffer index
volatile u32 r_flag,time,buf_index;

/**
 * @brief Initializes all necessary peripherals for the ATM system.
 * This includes UART for PC/server communication, LCD for display,
 * and Keypad Matrix for user input.
 */
void sys_init(void){
	// Initialize UART units (UART0 and UART1).
	// UART0 is typically for PC communication, UART1 for RFID.
	initUART(U0);
	initUART(U1);
	initLcd();	 	// Initialize the LCD display
	initKpm4x4();	// Initialize the 4x4 keypad matrix
	// Buzzer initialization (commented out, implying not implemented in this version)
					//buzzer
}

/*
// Original UART0_isr commented out.
// This was likely a basic receive interrupt handler.
void UART0_isr(void) __irq
{
//	VICIntEnable &= ~(1 << 6);
  if((U0IIR & 0x04)) //check if receive interrupt
  {
	  getMsg(buf);
	  if(!strcmp(buf,"@Y:LINEOK$")){
	  	sendMsg("#Y:LINEOK$");		
	  }else{
	  	r_flag=1;
	  }
  }
  dummy=U0IIR; //Read to Clear transmit interrupt
  VICVectAddr = 0;
 // VICIntEnable = 1 << 6;
}
*/

/**
 * @brief UART0 Interrupt Service Routine (ISR).
 * This ISR handles incoming data on UART0, primarily for communication with a PC/server.
 * It reads characters until a newline is received, processes specific "LINEOK" messages,
 * and sets a flag when a complete message is received.
 */
void UART0_isr(void) __irq {
    // Check if the interrupt is due to Receive Data Available (RDA)
    if ((U0IIR & 0x0E) == 0x04) { // U0IIR bit 2 (0x04) indicates RDA interrupt
        char ch = U0RBR; // Read the received character from the Receiver Buffer Register
        if (!r_flag){ // If the receive flag is not set (meaning we are ready for a new message)
            if(ch=='\n'){ // Check if the received character is a newline
		    buf[buf_index-1] = '\0';// Null-terminate the buffer (overwriting the '\r' if present)
		    // Check for a specific "LINEOK" message from the PC
		    if(!strcmp((const char*)buf,"@Y:LINEOK$")){
			    sendMsg("#Y:LINEOK$"); // Respond with a LINEOK message
		    }else{
			    r_flag=1; // Set receive flag to indicate a message has been received
		    }
		    buf_index=0; // Reset buffer index for the next message
	    } else if(buf_index < BUF_MAX-1) { // If not newline and buffer not full
                buf[buf_index++]=ch; // Store character in buffer and increment index
            }
        }
    }

    dummy = U0IIR;     // Read U0IIR to clear the interrupt (acknowledgment)
    VICVectAddr = 0;   // Acknowledge the interrupt to the VIC (Vector Interrupt Controller)
}

/**
 * @brief Sends a null-terminated string to the PC/server via UART0.
 * Appends a carriage return and newline for proper line termination.
 * Also displays the transmitted message on the LCD in debug mode.
 *
 * @param str Pointer to the null-terminated string to send.
 */
void sendMsg(s8 *str){
	strTxUART(U0,str);     // Transmit the string via UART0
	strTxUART(U0,"\r\n"); // Transmit carriage return and newline
#ifdef DBG // If DEBUG macro is defined
	moveLcdCursor(1,0);	   // Move LCD cursor to row 1, column 0
	str2Lcd("Tx:");       // Display "Tx:"
	str2Lcd(str);          // Display the transmitted string
	str2Lcd(".");          // Display a period
	delayS(1);             // Short delay for readability
#endif
}

/**
 * @brief Receives a null-terminated string from the PC/server via UART0.
 * Reads characters until a newline character is received.
 * Removes the trailing carriage return if present.
 * Also displays the received message on the LCD in debug mode.
 *
 * @param str Pointer to the buffer where the received string will be stored.
 */
void getMsg(s8 *str){
	// Includes '\r' (carriage return) typically when reading lines from a terminal
	strRx_UART(U0,str);
	// Removes '\r' (carriage return) by overwriting it with null terminator
	str[strlen(str)-1]='\0';
#ifdef DBG // If DEBUG macro is defined
	moveLcdCursor(1,0);	   // Move LCD cursor to row 1, column 0
	str2Lcd("Rx:");       // Display "Rx:"
	str2Lcd(str);          // Display the received string
	str2Lcd(".");          // Display a period
	delayS(1);             // Short delay for readability
#endif
}

/**
 * @brief Checks if a received message is in the expected format (starts with '@' and ends with '$').
 *
 * @param str Pointer to the message string to check.
 * @return 1 if the message format is valid, 0 otherwise.
 */
s32 isMsgOk(s8 *str){
	// Check if the first character is '@' and the last character is '$'
	if((str[0]!='@')||(str[strlen(str)-1]!='$'))return 0;
	return 1;
}

/**
 * @brief Prompts the user to enter a 4-digit PIN using the keypad.
 * Displays entered digits as '*' for security. Handles backspace and cancel ('C') input.
 * Implements a session timeout.
 *
 * @param str Pointer to a buffer where the entered PIN will be stored.
 * @return 1 if a 4-digit PIN is successfully entered, 0 if timed out, -1 if cancelled.
 */
u32 getPin(s8 *str){
	s32 i=0;     // Current digit index
	u32 time=ATM_TIME; // Session timeout counter
	s8 ch;       // Character read from keypad
	clearLcdRow(1); // Clear the second row of the LCD

	while(i<4 && time){ // Loop until 4 digits are entered or time runs out
		// Wait for a key press from the keypad
		while(!(ch=readKpm4x4())){
			if(time)--time; // Decrement time if no key pressed
			else return 0;  // Return 0 if timeout
		}
		// If a digit is pressed
		if(ISNUM(ch)){
			char2Lcd(ch);     // Display the actual digit temporarily (for a short time)
			delayMs(300);     // Short delay for user to see the digit
			moveLcdCursor(1,i); // Move cursor back to the position of the just-entered digit
			char2Lcd('*');    // Replace the digit with '*' for security
			str[i++]=ch;      // Store the digit in the buffer
			time=ATM_TIME;    // Reset session timeout
		}
		// If backspace ('\b') is pressed
		else if(ch=='\b'){
			if(i){ // If there are digits to delete
				--i;             // Decrement index
				moveLcdCursor(1,i); // Move cursor back
				char2Lcd(' ');   // Clear the '*' character
				moveLcdCursor(1,i); // Move cursor back again
			}
			time=ATM_TIME;       // Reset session timeout
		}
		// If 'C' (Cancel) is pressed
		else if(ch=='C'){
			return -1; // Return -1 for cancellation
		}
	}
	str[i]='\0'; // Null-terminate the PIN string

	return time?1:0; // Return 1 if time remains (successful entry), 0 if timed out
}

/**
 * @brief Reads a string input from the keypad and displays it on the LCD.
 * Handles numeric input, backspace, and "Enter" ('\n')/'Cancel' ('C') keys.
 * Implements a session timeout.
 *
 * @param str Pointer to the buffer where the entered string will be stored.
 * @param row The LCD row on which to display the input.
 * @return 0 if a valid string is entered and confirmed, 1 if timed out, -1 if cancelled.
 */
int str_KPM(s8 *str,u8 row){
	u32 i=0;         // Current character index
	s8 ch='\0';      // Character read from keypad
	moveLcdCursor(row,0); // Move LCD cursor to the specified row, column 0

	// Loop until buffer is full or session times out
	while(i<BUF_MAX-1 && time){
		ch=readKpm4x4(); // Read character from keypad
		if(ch)time=ATM_TIME; // Reset session timeout if a key is pressed
		else{
			time--;          // Decrement time if no key pressed
			continue;        // Continue loop
		}
		// If backspace ('\b') is pressed
		if(ch=='\b'){
			str[i]='\0';         // Null-terminate the string at current position
			moveLcdCursor(row,i); // Move cursor to current position
			char2Lcd(' ');       // Clear the character
			moveLcdCursor(row,i); // Move cursor back
			if(i)--i;            // Decrement index if not at the beginning
		}
		// If Enter ('\n') is pressed and some characters have been entered
		else if(i && ch=='\n'){
			str[i]='\0';         // Null-terminate the string
			return 0;            // Return 0 for valid string + enter
		}
		// If a numeric digit is pressed
		else if(ISNUM(ch)){
			if(!i && (ch=='0'))continue; // Prevent leading zeros for first digit
			str[i]=ch;           // Store the digit
			char2Lcd(ch);        // Display the digit
			i++;                 // Increment index
		}
		// If 'C' (Cancel) is pressed
		else if(ch=='C'){
		 	return -1;           // Return -1 for cancellation
		}
	}
	// Session time-out if loop exits due to time
	return 1;
}

/**
 * @brief Handles the "Withdraw Cash" functionality.
 * Prompts for amount, validates it (multiples of 100), and sends a withdraw request to the PC.
 * Interprets the PC's response and displays appropriate messages.
 *
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to a buffer for sending/receiving messages.
 */
void atm_wtd(s8 *rfid,s8 *buf){
	s8 amt[20]; // Buffer for amount input
	clearLcdDisplay(); // Clear LCD
	str2Lcd("Withdraw amt:"); // Prompt for amount

	// Get amount input from keypad
	if(str_KPM(amt,1)==-1){ // If cancelled
		moveLcdCursor(0,0);
		str2Lcd(" Cancelled !!!! ");
		clearLcdRow(1);
		delayS(2);
		return;		
	}
	// Validate amount: must be multiples of 100
	if((amt[strlen(amt)-2]!='0')||(amt[strlen(amt)-1]!='0')){
		moveLcdCursor(0,0);
		str2Lcd(" Amount Must be ");
		moveLcdCursor(1,0);
		str2Lcd("100Rs multiples!");
		delayS(2);
		return;
	}
	// Try to withdraw
	checkPC(); // Ensure PC connection is active
	// Format the withdraw request message: #A:WTD:<rfid>:<amt>$
	sprintf(buf,"#%c:WTD:%s:%s$",'A',rfid,amt);
	sendMsg(buf); // Send the message to PC

#ifdef UART_INT // If UART interrupt is enabled
	while(!r_flag); // Wait for response
	r_flag=0;       // Clear receive flag
#else
	getMsg(buf); // Get message from PC (blocking)
#endif
	// Interpret PC's response: @OK:DONE$, @ERR:LOWBAL$, @ERR:NEGAMT$, @ERR:MAXAMT$
	if(!strcmp(buf,"@OK:DONE$")){
		moveLcdCursor(0,0);
		str2Lcd("Amount Withdrawn");
		moveLcdCursor(1,0);
		str2Lcd("  Succesfully!! ");
		delayS(2);
	}else if(!strcmp(buf,"@ERR:LOWBAL$")){
		moveLcdCursor(0,0);
		str2Lcd("Withdraw Failed ");
		moveLcdCursor(1,0);
		str2Lcd("  Low-Balance!  ");
	}else if(!strcmp(buf,"@ERR:NEGAMT$")){
		// Handle negative withdraw amount error (message not explicitly defined, but logic exists)
		moveLcdCursor(0,0);
		str2Lcd("Withdraw Failed ");
		moveLcdCursor(1,0);
		str2Lcd("  Negative Amt! ");
	}else if(!strcmp(buf,"@ERR:MAXAMT$")){
		moveLcdCursor(0,0);
		str2Lcd("Withdraw Failed ");
		moveLcdCursor(1,0);
		str2Lcd("Exceeds MaxLimit");
	}else{
		// Handle wrong content/unknown error
		moveLcdCursor(0,0);
		str2Lcd("  Unknown Error ");
		moveLcdCursor(1,0);
		str2Lcd(" during Withdraw");
	}
	delayS(2);
}

/**
 * @brief Handles the "Deposit Cash" functionality.
 * Prompts for amount, and sends a deposit request to the PC.
 * Interprets the PC's response and displays appropriate messages.
 *
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to a buffer for sending/receiving messages.
 */
void atm_dep(s8 *rfid,s8 *buf){
	s8 amt[20]; // Buffer for amount input
	clearLcdDisplay(); // Clear LCD
	str2Lcd("Deposit amt:"); // Prompt for amount

	// Take amount input from keypad
	if(str_KPM(amt,1)==-1){ // If cancelled
		moveLcdCursor(0,0);
		str2Lcd(" Cancelled !!!! ");
		clearLcdRow(1);
		delayS(2);
		return;		
	}
	// Try to deposit
	checkPC(); // Ensure PC connection is active
	// Format the deposit request message: #A:DEP:<rfid>:<amt>$
	sprintf(buf,"#%c:DEP:%s:%s$",'A',rfid,amt);
	sendMsg(buf); // Send message to PC

#ifdef UART_INT // If UART interrupt is enabled
	while(!r_flag); // Wait for response
	r_flag=0;       // Clear receive flag
#else	
	getMsg(buf); // Get message from PC (blocking)
#endif
	// Interpret PC's response: @OK:DONE$, @ERR:NEGAMT$, @ERR:MAXAMT$
	if(!strcmp(buf,"@OK:DONE$")){
		moveLcdCursor(0,0);
		str2Lcd("Amount Deposited");
		moveLcdCursor(1,0);
		str2Lcd("  Succesfully!! ");
		
	}else if(!strcmp(buf,"@ERR:NEGAMT$")){
		// Handle negative deposit amount error
		moveLcdCursor(0,0);
		str2Lcd("Deposit Failed ");
		moveLcdCursor(1,0);
		str2Lcd("  Negative Amt! ");
	}else if(!strcmp(buf,"@ERR:MAXAMT$")){
		moveLcdCursor(0,0);
		str2Lcd("Deposit Failed ");
		moveLcdCursor(1,0);
		str2Lcd("Exceeds MaxLimit");
		
	}else{
		// Handle wrong content/unknown error
		moveLcdCursor(0,0);
		str2Lcd("  Unknown Error ");
		moveLcdCursor(1,0);
		str2Lcd(" during Deposit ");
	}
	delayS(2);
}

/**
 * @brief Handles the "View Balance" functionality.
 * Sends a balance inquiry request to the PC.
 * Extracts and displays the balance from the PC's response.
 *
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to a buffer for sending/receiving messages.
 */
void atm_bal(s8 *rfid,s8 *buf){
	s32 i=0; // Loop counter
	clearLcdDisplay(); // Clear LCD
	str2Lcd("Balance:"); // Display "Balance:"

	checkPC(); // Ensure PC connection is active
	while(1){ // Loop until a valid message is received
		// Format the balance inquiry message: #A:BAL:<rfid>$
		sprintf(buf,"#%c:BAL:%s$",'A',rfid);
		sendMsg(buf); // Send message to PC
#ifdef UART_INT // If UART interrupt is enabled
		while(!r_flag); // Wait for response
		r_flag=0;       // Clear receive flag
#else
		getMsg(buf); // Get message from PC (blocking)
#endif
		// Check if received message format is valid
		if(isMsgOk(buf))break;
	}
	
	// Interpret PC's response: @OK:BAL=<amt>$
	if(!strncmp(buf,"@OK:BAL=",7)){ // Check for "@OK:BAL=" prefix
		// Extract balance amount from the message
		i=0;
		while(1){
			if(buf[i+8]=='$'){ // Stop when '$' is encountered
				buf[i]='\0';   // Null-terminate the extracted balance string
				break;
			}
			buf[i]=buf[i+8]; // Copy balance digits to the beginning of the buffer
			i++;
		}
		// Display the balance
		clearLcdRow(1); // Clear the second row
		str2Lcd(buf);   // Display the extracted balance
		str2Lcd(" Rs");   // Display " Rs"
		delayS(2);
	}else{
		// Handle wrong content/unknown error
		moveLcdCursor(0,0);
		str2Lcd("  Unknown Error ");
		moveLcdCursor(1,0);
		str2Lcd(" during Balance ");
	}
}

/**
 * @brief Handles the "PIN Change" functionality.
 * Prompts for old PIN, new PIN, and re-entry of new PIN.
 * Verifies old PIN, validates new PINs, and sends a PIN change request to the PC.
 * Updates the local PIN if successful.
 *
 * @param rfid Pointer to the RFID tag number string.
 * @param pin Pointer to the current PIN string (will be updated if change is successful).
 * @param buf Pointer to a buffer for sending/receiving messages.
 * @return 1 if PIN change is successful, 0 otherwise.
 */
int  atm_pin( s8 *rfid,s8 *pin,s8 *buf){
	s8 dum[5]={0}; // Temporary buffer for re-entered new PIN
	s32 ret;      // Return value from getPin

	clearLcdDisplay(); // Clear LCD
	str2Lcd("Enter Old pin:"); // Prompt for old PIN

	// Get old PIN from keypad
	ret=getPin(buf);
	if(!ret){ // If timed out
		moveLcdCursor(0,0);
		str2Lcd("Session Time-Out");
		moveLcdCursor(1,0);
		str2Lcd("   Thank you.   ");
		delayS(2);
		return 0;
	}else if(ret==-1){ // If cancelled
		moveLcdCursor(0,0);
		str2Lcd(" Cancelled !!!! ");
		clearLcdRow(1);
		delayS(2);
		return 0;		
	}
	// Check if old PIN matches the current PIN
	if(strcmp(pin,buf)){ // If not matched
		clearLcdDisplay();
		str2Lcd("Incorrect pin!!");
		delayS(1);
		return 0; // Return 0 for incorrect old PIN
	}

	// Prompt for new PIN
	clearLcdDisplay();
	str2Lcd("Enter New pin:");
	ret=getPin(buf); // Get new PIN from keypad
	if(!ret){ // If timed out
		moveLcdCursor(0,0);
		str2Lcd("Session Time-Out");
		moveLcdCursor(1,0);
		str2Lcd("   Thank you.   ");
		delayS(2);
		return 0;
	}else if(ret==-1){ // If cancelled
		moveLcdCursor(0,0);
		str2Lcd(" Cancelled !!!! ");
		clearLcdRow(1);
		delayS(2);
		return 0;		
	}
	// Prompt to re-enter new PIN for verification
	clearLcdDisplay();
	str2Lcd("Re-enter New pin");
	ret=getPin(dum); // Get re-entered new PIN
	if(!ret){ // If timed out
		moveLcdCursor(0,0);
		str2Lcd("Session Time-Out");
		moveLcdCursor(1,0);
		str2Lcd("   Thank you.   ");
		delayS(2);
		return 0;
	}else if(ret==-1){ // If cancelled
		moveLcdCursor(0,0);
		str2Lcd(" Cancelled !!!! ");
		clearLcdRow(1);
		delayS(2);
		return 0;		
	}
	// Verify if new PIN and re-entered new PIN match
	if(strcmp(buf,dum)){ // If mismatch
		clearLcdDisplay();
		str2Lcd("New pin mismatch");
		delayS(1);
		return 0; // Return 0 for mismatch
	}
	strcpy(pin,dum); // Update the local PIN with the new PIN

	// Send PIN change request to PC
	checkPC(); // Ensure PC connection is active
	while(1){ // Loop until a valid response is received
		// Format PIN change message: #A:PIN:<rfid>:<pin>$
		sprintf(buf,"#%c:PIN:%s:%s$",'A',rfid,pin);
		sendMsg(buf); // Send message to PC
#ifdef UART_INT // If UART interrupt is enabled
		while(!r_flag); // Wait for response
		r_flag=0;       // Clear receive flag
#else
		getMsg(buf); // Get message from PC (blocking)
#endif
		// Check if received message format is valid
		if(isMsgOk(buf))break;
	}
	
	// Interpret PC's response: @OK:DONE$
	if(!strcmp(buf,"@OK:DONE$")){
		return 1; // Return 1 for successful PIN change
	}else{
		clearLcdDisplay();
		str2Lcd("Unknown Error");
		return 0; // Return 0 for unknown error
	}
}

/**
 * @brief Handles the "Mini Statement" functionality.
 * Retrieves and displays recent transactions from the PC, one by one.
 * Displays transaction type (WTD, DEP, TIN, TOT) and amount/date.
 *
 * @param rfid Pointer to the RFID tag number string.
 * @param buf Pointer to a buffer for sending/receiving messages.
 */
void atm_mst(s8 *rfid,s8 *buf){
	s32 i=0,j=0; // Loop counters
	// Array of transaction type strings (WTD=Withdraw, DEP=Deposit, TIN=Transfer In, TOT=Transfer Out)
	s8 t[][4]={"WTD","DEP","TIN","TOT"};
	
	// Get transactions one by one (up to 3 transactions for mini statement)
	checkPC(); // Ensure PC connection is active
	while(j<3){ // Loop for up to 3 transactions
		// Format Mini Statement request: #A:MST:<rfid>:<txNo>$
		sprintf(buf,"#%c:MST:%s:%d$",'A',rfid,j+1); // j+1 for transaction number
		sendMsg(buf); // Send message to PC
		//@:TXN:<type>:<amt>$ (Expected response format)
#ifdef UART_INT // If UART interrupt is enabled
		while(!r_flag); // Wait for response
		r_flag=0;       // Clear receive flag
#else
		getMsg(buf); // Get message from PC (blocking)
#endif
		// Check if received message format is valid
		if(isMsgOk(buf)){
		 	// Check if no more transactions are available (buf[5] is '7' for no transactions)
			if(buf[5]=='7'){
			 	break; // Exit loop if no more transactions
			}else{
				// Expected format: "@TXN:type:date time:amount$"
				// Example: "@TXN:1:15/05/2024 10:30:123.45$"
				moveLcdCursor(0,0);
				// Display transaction date and time (assumed to be from buf[7] for 16 chars)
				for(i=0;i<16;i++){
					char2Lcd(buf[i+7]);
				}
				clearLcdRow(1); // Clear second row
				moveLcdCursor(1,0);
				// Display transaction amount (assumed to be from buf[24] until '$')
				for(i=0;buf[i+24]!='$';i++){
					char2Lcd(buf[i+24]);
				}
				moveLcdCursor(1,13); // Move cursor to right for transaction type
				str2Lcd(t[buf[5]-'1']); // Display transaction type string (convert char '1'-'4' to index 0-3)
				delayS(2); // Short delay
			}
			 ++j; // Increment transaction counter
		}
	}
}

/**
 * @brief Checks and maintains the connection with the PC/server.
 * Sends "LINEOK" messages and waits for a corresponding response.
 * Blocks until connection is established.
 */
void checkPC(void){
	while(1){ // Loop indefinitely until PC connection is confirmed
		sendMsg("#X:LINEOK$"); // Send "LINEOK" message to PC
#ifdef UART_INT // If UART interrupt is enabled
		while(!r_flag); // Wait for response
		r_flag=0;       // Clear receive flag
#else
		getMsg(buf); // Get message from PC (blocking)
#endif
			/*
			clearLcdDisplay();
			moveLcdCursor(0,0);
			str2Lcd("~");
			str2Lcd((const char*)buf);
			char2Lcd('~');
			char2Lcd(strlen((const char*)buf)+'a');
			delayS(2);
			*/
			
		// Check if PC responded with "@X:LINEOK$"
		if(!strcmp((const char*)buf,"@X:LINEOK$")){
		/*
			moveLcdCursor(1,0);
			str2Lcd("Pc line ok bro .");
		*/
			break; // Exit loop if PC connection is OK
		
		}else{
			moveLcdCursor(1,0);
			str2Lcd("waiting for PC."); // Display waiting message
		}
		delayS(2); // Short delay before retrying
	}
}
