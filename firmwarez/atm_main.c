// atm_main.c - Main application file for ATM system
#include "atmLib.h" // Include the header file for ATM library functions

// Array of strings for the ATM menu options
const s8 menu[6][16]={"1.WITHDRAW CASH","2.DEPOSIT CASH","3.VEIW BALANCE","4.MINI STATMENT","5.PIN CHANGE","6.EXIT ATM"};

/**
 * @brief Main function of the ATM application.
 * Initializes the system, handles RFID card reading, PIN verification,
 * and navigates through the ATM menu operations.
 * Implements session management, retry limits for PIN, and card blocking.
 *
 * @return 0 upon successful execution (though typically an embedded system loops indefinitely).
 */
s32 main(){

	s8 rfid[9]="11111111",pin[5]="1111",ch; // Buffers for RFID, PIN, and keypad character
	s32 trys,curRow=0,prevRow=-1,errNo;     // Variables for retry count, current menu row, previous menu row, and error number
	sys_init(); // Initialize all system peripherals (UART, LCD, Keypad)

	// Welcome screen display
	moveLcdCursor(0,0);
	str2Lcd(" Welcome To ATM ");
	sendMsg(" Welcome To ATM "); // Send welcome message to PC (for logging/debug)
	delayS(1); // Short delay

	//checkPC(); // Ensure PC connection is established (uncommented for testing)
	//goto A;//testing :) // Jump for testing purposes, bypasses card/PIN authentication

	// Super loop: Main operational loop of the ATM
	while(1){
		// Prompt to place RFID card
		moveLcdCursor(0,0);
		str2Lcd("Place the card  ");
		moveLcdCursor(1,0);
		str2Lcd("on the reader.. ");

		// Wait for RFID card data
		getRFID(buf); // Read RFID data into 'buf'
		
		// Validate RFID card data
		if(!isRfidOk(buf)){
		 // If RFID data is not OK, prompt to place card again
		 delayS(1);
		 continue; // Restart loop
		}
		
		// Extract RFID number from the received data
		// RFID data format is typically <STX><8-digit RFID><ETX> (e.g., 0x02"12345678"0x03)
		strncpy(rfid,(const char*)buf+1,8); // Copy 8 digits starting after STX (buf[0])
		rfid[8]='\0'; // Null-terminate the RFID string

		clearLcdDisplay(); // Clear LCD
		str2Lcd("RFID:"); // Display "RFID:"
		str2Lcd(rfid); // Display extracted RFID number

		// Check RFID status with PC
		checkPC(); // Ensure PC connection is active
		
		while(1){ // Loop until a valid response for card status is received
			// Format card check message: #C:<rfid>$
			sprintf(buf,"#%c:%s$",'C',rfid);
			sendMsg(buf); // Send message to PC
#ifdef UART_INT // If UART interrupt is enabled
			while(!r_flag); // Wait for response from PC
			r_flag=0;       // Clear receive flag
#else
			getMsg(buf); // Get message from PC (blocking)
#endif
			// Check if received message format is valid
			if(isMsgOk(buf))break;
		}

		// Interpret PC's response for card status: @OK:ACTIVE:<name>$, @ERR:INVALID$, @ERR:BLOCK$
		if(!strncmp((const char*)buf,"@OK:ACTIVE:",11)){ // If card is active
			moveLcdCursor(0,0);
			str2Lcd("Welcome customer"); // Display welcome message
			moveLcdCursor(1,0);
			// Display customer name (extract from message after "@OK:ACTIVE:")
			for(time=0;(buf[time+11]!='$')&&(time<16);time++){ // Loop to display name up to 16 chars or '$'
				char2Lcd(buf[time+11]);
			}
			for(;time<16;time++)char2Lcd(' '); // Fill remaining space with spaces
			delayS(2); // Short delay
		}else if(!strcmp((const char*)buf,"@ERR:BLOCK$")){ // If card is blocked
			moveLcdCursor(0,0);
			str2Lcd("Card is Blocked!");
			moveLcdCursor(1,0);
			str2Lcd("   Visit Bank.  ");
			delayS(2);
			continue; // Restart super loop (prompt for new card)
		}else if(!strcmp((const char*)buf,"@ERR:INVALID$")){ // If card not found
			moveLcdCursor(0,0);
			str2Lcd(" Card not Found!");
			moveLcdCursor(1,0);
			str2Lcd("Register at bank");
			delayS(2);
			continue; // Restart super loop
		}else{
			// Handle wrong content/unknown response from PC
			continue; // Restart super loop
		}
			
		// Take PIN and verify
		trys=MAX_TRYS; // Initialize PIN retry count
		while(trys){ // Loop for PIN attempts
			if(trys!=MAX_TRYS){ // If not the first attempt
				moveLcdCursor(0,0);
				str2Lcd("  Wrong Pin!!!  ");
				moveLcdCursor(1,0);
				char2Lcd('0'+trys); // Display remaining tries
				str2Lcd(" - Tries Left  ");
				delayS(2);
			}
			moveLcdCursor(0,0);
			str2Lcd("Enter pin:      ");
			clearLcdRow(1); // Clear second row for PIN input

			// Get PIN from keypad
			errNo=getPin(pin); // 'errNo' stores the result: 1=success, 0=timeout, -1=cancel
			if(!errNo){ // If timed-out
				moveLcdCursor(0,0);
				str2Lcd("Session Time-Out");
				moveLcdCursor(1,0);
				str2Lcd("   Thank you.   ");
				delayS(2);
				break; // Exit PIN entry loop, go back to card placement
			}else if(errNo==-1){ // If cancelled
				moveLcdCursor(0,0);
                str2Lcd("Session Canceled");
                moveLcdCursor(1,0);
                str2Lcd("   Thank you.   ");
                delayS(2);
                break; // Exit PIN entry loop
			}
			// Verify PIN with PC
			checkPC(); // Ensure PC connection
			while(1){ // Loop until a valid response for PIN verification
				sprintf(buf,"#%c:%s:%s$",'V',rfid,pin); // Format PIN verification message: #V:<rfid>:<pin>$
				sendMsg(buf); // Send message to PC
#ifdef UART_INT
				while(!r_flag);
				r_flag=0;
#else
				getMsg(buf);
#endif
				if(isMsgOk(buf))break;
			}
			
			// Interpret PC's response for PIN verification: @OK:MATCHED$, @ERR:WRONG$
		 	if(!strcmp((const char*)buf,"@OK:MATCHED$")){
		 		break; // PIN matched, exit PIN entry loop
		 	}else if(!strcmp((const char*)buf,"@ERR:WRONG$")){
				trys--; // PIN wrong, decrement tries
		 	}else{
				// Wrong content/unknown response
				continue;
			}
		}
		if(errNo!=1)continue; // If PIN entry was not successful (timeout/cancel), restart super loop

		
		if(!trys){ // If no tries left (PIN attempts exhausted)
			// Card blocked due to too many wrong PIN attempts
			while(1){ // Loop to send card block message until acknowledged
				sprintf(buf,"#%c:BLK:%s$",'A',rfid); // Format block message: #A:BLK:<rfid>$
				sendMsg(buf);
#ifdef UART_INT
				while(!r_flag);
				r_flag=0;
#else
				getMsg(buf);
#endif		
				if(isMsgOk(buf)){
					if(!strcmp((const char*)buf,"@OK:DONE$")) break; // Acknowledged, exit loop
				}
			}
			clearLcdDisplay();
			str2Lcd(" Card Blocked!! ");
			moveLcdCursor(1,0);
			str2Lcd("   Visit Bank.  ");
			delayS(2);
			continue; // Restart super loop
		}
A: // Label for skipping authentication (used with goto for testing)
		// ATM operational start (after successful authentication)
		prevRow=-1; // Reset previous menu row to force full menu redraw
		curRow=0;   // Start at first menu option
		trys=MAX_TRYS; // Reset tries for PIN change attempts
		time=ATM_TIME; // Reset session time-out
		while(time){ // Loop for ATM session until timeout or exit
			// ATM menu scrolling and display
			if(curRow!=prevRow){ // If current row is different from previous (menu changed)
				clearLcdDisplay(); // Clear display
				str2Lcd(menu[curRow]); // Display current menu option on row 0
				moveLcdCursor(1,0);
				str2Lcd(menu[curRow+1]); // Display next menu option on row 1
				prevRow=curRow; // Update previous row
			}
			// Wait for keypad input
			while(!(ch=readKpm4x4())){
				if(time)--time; // Decrement time if no key pressed
				else break;     // Break loop if session timeout
			}
			
			if(ch){ // If a key is pressed
				if(ch=='B'){ // 'B' for scroll down
					if(curRow<6-2)curRow++; // Scroll down if not at last two options (to ensure two options are always visible)
				}else if(ch=='A'){ // 'A' for scroll up
					if(curRow)curRow--; // Scroll up if not at first option
				}
				// If a numeric key matching a displayed menu option is pressed
				else if((ch==('0'+curRow+1))||(ch==('0'+curRow+2))){
					// Perform actions based on selected option
					if(ch=='1'){
						// Withdraw cash
						atm_wtd(rfid,buf);
						prevRow=-1; // Reset prevRow to force menu redraw
					}else if(ch=='2'){
						// Deposit cash
						atm_dep(rfid,buf);
						prevRow=-1;
					}else if(ch=='3'){
						// View balance
						atm_bal(rfid,buf);
						delayS(3); // Display balance for a longer period
						prevRow=-1;
					}else if(ch=='4'){
						// Mini statement
						atm_mst(rfid,buf);
						prevRow=-1;
					}else if(ch=='5'){
						// PIN change
						if(trys){ // Check if PIN change tries left
							// Attempt PIN change
							if(atm_pin(rfid,pin,buf)){ // If PIN change successful
								clearLcdDisplay();
								str2Lcd("Pin changed.");
								trys=MAX_TRYS; // Reset PIN change tries
							}else{ // If PIN change failed
								moveLcdCursor(1,0);
								str2Lcd("Trys left: ");
								--trys; // Decrement PIN change tries
								char2Lcd(trys+'0'); // Display remaining tries
							}
						}
						delayMs(DISP_TIME); // Short display time for messages
						if(!trys){ // If no PIN change tries left
							// Block card and exit session
							while(1){
								sprintf(buf,"#%c:BLK:%s$",'A',rfid);
								sendMsg(buf);
#ifdef UART_INT
								while(!r_flag);
								r_flag=0;
#else
								getMsg(buf);
#endif		
								if(isMsgOk(buf)){
									if(!strcmp((const char*)buf,"@OK:DONE$")) break;
								}
							}
							moveLcdCursor(0,0);
							str2Lcd(" Card Blocked!! ");
							moveLcdCursor(1,0);
							str2Lcd("   Visit Bank.  ");
							delayS(2);
							break; // Exit ATM session loop
						}
						prevRow=-1; // Reset prevRow
					}else if(ch=='6'){
						// Exit ATM
						checkPC(); // Ensure PC connection
						sendMsg("#Q:SAVE$"); // Send save/quit command to PC
						prevRow=-1;
						break; // Exit ATM session loop
					}
				}
				time=ATM_TIME; // Reset session time-out after any valid key press
			}
			//
			if(time)--time; // Decrement session time (if no key pressed in current cycle)
		}
		if(time){ // If session ended due to user exiting (time > 0)
			moveLcdCursor(0,0);
            str2Lcd("  Thank You !!  ");
            moveLcdCursor(1,0);
            str2Lcd("Have a nice Day.");
		}else{ // If session ended due to inactivity (time == 0)
				//atm closed due to inactivity
			moveLcdCursor(0,0);
			str2Lcd("Session Time-Out");
			moveLcdCursor(1,0);
			str2Lcd("   Thank you.   ");
		}
		delayS(2); // Short delay before restarting the super loop (card placement screen)
		//
	}
	//
	return 0; // Main function returns 0
}
