#include "atmLib.h" //Includes the atmLib.h header file for function declarations, structures, and macros

#define BAUD B9600 //Defines the baud rate for serial communication (9600 bps)

/**
 * @brief Initializes the serial port (/dev/ttyUSB0) for communication.
 * Configures the port to raw mode, sets baud rate, enables local connection and reading,
 * sets 8 data bits, no parity, 1 stop bit, and disables canonical mode, echo, and signal chars.
 * Makes read a blocking function.
 * @param void No parameters.
 * @return int File descriptor for the opened serial port. Exits program on failure to open.
 */
int initSerial(void){

        int fd; //File descriptor for the serial port
        struct termios opt; //Structure to hold terminal attributes
        //Comment indicating the start of serial port opening logic
        //fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY| O_NDELAY); //Alternative open call with non-blocking reads (commented out)
        fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY); //Opens the serial port /dev/ttyUSB0 for read/write, not as controlling terminal
        if(fd==-1){ //Checks if the port opening failed
                perror("open"); //Prints the system error message for "open"
                exit(1); //Exits the program with status 1 (error)
        }
        //make read a blocking func //Comment indicating configuration for blocking read
        fcntl(fd,F_SETFL,0); //Sets file status flags for 'fd'; 0 makes read() blocking
        // Get and modify current options: //Comment indicating the process of getting and setting terminal attributes

        tcgetattr (fd, &opt) ; //Gets the current terminal attributes for 'fd' and stores them in 'opt'

        cfmakeraw   (&opt) ; //Sets the terminal to raw mode (non-canonical, no echo, etc.)
        cfsetispeed (&opt, BAUD) ; //Sets the input baud rate
        cfsetospeed (&opt, BAUD) ; //Sets the output baud rate

        opt.c_cflag |= (CLOCAL | CREAD) ; //Enables local connection (ignore modem control lines) and enables receiver
        opt.c_cflag &= ~PARENB ; //Disables parity generation and detection
        opt.c_cflag &= ~CSTOPB ; //Sets one stop bit (instead of two)
        opt.c_cflag &= ~CSIZE ; //Clears the character size bits
        opt.c_cflag |= CS8 ; //Sets character size to 8 bits
        opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ; //Disables canonical mode, echo, echo erase, and signal characters
        opt.c_oflag &= ~OPOST ; //Disables implementation-defined output processing

        tcsetattr (fd, TCSANOW | TCSAFLUSH, &opt) ; //Sets the modified terminal attributes immediately and flushes pending I/O

        usleep (10000) ;        //Pauses for 10 milliseconds to allow settings to take effect
        return fd; //Returns the file descriptor of the initialized serial port
}

/**
 * @brief Flushes both input and output buffers of the serial port.
 * @param fd The file descriptor of the serial port.
 * @param void No return value.
 */
void flushSerial (const int fd)
{
        tcflush (fd, TCIOFLUSH) ; //Discards data written but not transmitted and data received but not read
}

/**
 * @brief Closes the serial port.
 * @param fd The file descriptor of the serial port to be closed.
 * @param void No return value.
 */
void endSerial(const int fd){
        close(fd); //Closes the file descriptor associated with the serial port
}

/**
 * @brief Transmits a single character over the serial port.
 * @param fd The file descriptor of the serial port.
 * @param ch The character to be transmitted.
 * @return int The number of bytes written (1 on success, -1 on error).
 */
int tx_char(const int fd,const char ch){
        return write(fd,&ch,1); //Writes one character 'ch' to the serial port 'fd'
}

/**
 * @brief Transmits a null-terminated string over the serial port, followed by a carriage return and newline (CR LF).
 * @param fd The file descriptor of the serial port.
 * @param str The null-terminated string to transmit.
 * @return int Returns the result of the last write operation (typically bytes written for CR LF, or error).
 * Note: The function performs two write calls.
 */
int tx_str(const int fd,const char *str){
        write(fd,str,strlen(str)); //Writes the string 'str' to the serial port 'fd'
        write(fd,"\r\n",2); //Writes carriage return and newline characters to the serial port
#ifdef DBG //Conditional compilation block for debugging
        printf("DBG_TX:%s\n",str); //Prints the transmitted string to the console if DBG is defined
#endif //End of DBG conditional block
}

/**
 * @brief Receives a single character from the serial port.
 * This is a blocking read.
 * @param fd The file descriptor of the serial port.
 * @return char The received character. Returns -1 if read fails to get 1 byte.
 */
char rx_char(const int fd){
        char ch; //Buffer for the received character
        if(read(fd,&ch,1)!=1)return -1; //Reads one character from 'fd' into 'ch'; returns -1 if 1 byte is not read
        return ch; //Returns the received character
}

/**
 * @brief Receives a string from the serial port until a newline character ('\n') is encountered
 * or the buffer length 'len' is almost reached. The newline and preceding carriage return
 * are replaced with a null terminator.
 * @param fd The file descriptor of the serial port.
 * @param str Pointer to the character array where the received string will be stored.
 * @param len The maximum number of characters to read (size of the 'str' buffer).
 * @param void No return value. Exits on read error.
 */
void rx_str(const int fd,char *str,size_t len){
        int i=0,j=0; //i: index for 'str' buffer, j: return value of read()
        char ch; //Buffer for each character read
        for(;i<len-1;i++){ //Loop to read characters up to len-1 or until newline
                j=read(fd,&ch,1); //Reads one character from the serial port
                if(j==-1){ //If read() returns an error
                        printf("i=%d\n",i); //Prints the current index 'i'
                        perror("read_str"); //Prints the system error message for "read_str"
                        exit(1); //Exits the program with status 1 (error)
                }else if(j==0){ //If read() returns 0 (no data, non-blocking scenario, though configured as blocking)
                        i--; //Decrement 'i' to retry reading for the current position
                        continue; //Continues to the next iteration of the loop
                }
                str[i]=ch; //Stores the read character in the buffer
                if(ch=='\n')break; //If the character is a newline, exit the loop
        }
        str[i-1]='\0';//Replaces the carriage return (assuming it's before '\n') with a null terminator
#ifdef DBG //Conditional compilation block for debugging
        printf("DBG_RX:%s\n",str); //Prints the received string to the console if DBG is defined
#endif //End of DBG conditional block

}

/**
 * @brief Checks if the received message string is in the expected format.
 * The format is checked by verifying if the first character is '#' and the last character is '$'.
 * @param buf Pointer to the character array containing the message.
 * @return int Returns 1 if the message format is okay, 0 otherwise.
 */
int isMsgOk(const char *buf){
        if((buf[0]=='#')&&(buf[strlen(buf)-1]=='$'))return 1; //Checks first and last characters for format markers
        return 0; //Returns 0 if format is not okay
}

/**
 * @brief Checks the provided RFID against the account database.
 * Extracts RFID from the buffer, searches for it, and sends a status message back via serial.
 * Message format: #C:<rfid>$
 * Response: @OK:ACTIVE:<username>$ or @ERR:BLOCK$ or @ERR:INVALID$
 * @param head Pointer to the head of the linked list of accounts.
 * @param fd File descriptor for serial communication.
 * @param buf Pointer to the received message buffer containing the RFID.
 * @param void No return value.
 */
void checkRFID(Acc *head,const int fd,const char *buf){
        //#C:<rfid>$ //Expected message format
        //check rfid in database
        char rfid[9]; //Buffer to store the extracted 8-character RFID + null terminator
        char temp[32]; //Buffer for formatting the response string
        strncpy(rfid,buf+3,8); //Copies 8 characters starting from buf[3] (after "#C:") into 'rfid'
        rfid[8]='\0'; //Null-terminates the rfid string
        Acc *usr=getAcc(head,rfid); //Searches for the account with the given rfid
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs a microcontroller connectivity check
#endif //End of INT conditional block
        if(usr){ //If the account (user) is found
                //card status check
                if(usr->cardStat){ //If the card status is active
                        sprintf(temp,"@OK:ACTIVE:%s$",usr->usrName); //Formats an "ACTIVE" response with username
                        tx_str(fd,temp); //Sends the formatted response
                }else{ //If the card status is not active (blocked)
                        tx_str(fd,"@ERR:BLOCK$"); //Sends a "BLOCKED" error response
                }
        }else{ //If the account (user) is not found
                tx_str(fd,"@ERR:INVALID$"); //Sends an "INVALID" error response
        }
}

/**
 * @brief Verifies the PIN for a given RFID.
 * Extracts RFID and PIN from the buffer, finds the account, compares PINs, and sends a status message.
 * Message format: #V:<rfid>:<pin>$
 * Response: @OK:MATCHED$ or @ERR:WRONG$
 * @param head Pointer to the head of the linked list of accounts.
 * @param fd File descriptor for serial communication.
 * @param buf Pointer to the received message buffer containing RFID and PIN.
 * @param void No return value.
 */
void verifyPin(Acc *head,const int fd,const char *buf){
        //#V:<rfid>:<pin>$ //Expected message format
        //verify rfid with pin
        char rfid[9]; //Buffer for extracted RFID (8 chars + null)
        char pin[5]; //Buffer for extracted PIN (4 chars + null)

        strncpy(rfid,buf+3,8); //Extracts RFID (8 chars from buf[3])
        rfid[8]='\0'; //Null-terminates RFID string
        strncpy(pin,buf+12,4); //Extracts PIN (4 chars from buf[12], after "<rfid>:")
        pin[4]='\0'; //Null-terminates PIN string

        Acc *usr=getAcc(head,rfid); //Retrieves the account associated with the RFID
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs a microcontroller connectivity check
#endif //End of INT conditional block
        if(!strcmp(pin,usr->pin)){ //Compares the extracted PIN with the stored PIN for the user
                tx_str(fd,"@OK:MATCHED$"); //If PINs match, sends "MATCHED" response

        }else{ //If PINs do not match
                tx_str(fd,"@ERR:WRONG$"); //Sends "WRONG" PIN error response

        }
}

/**
 * @brief Processes various ATM actions like withdrawal, deposit, balance inquiry, etc.
 * Extracts RFID, request type, and other data from the buffer, then calls appropriate sub-functions.
 * Supported requests and formats:
 * #A:WTD:<rfid>:<amt>$  (Withdraw)
 * #A:DEP:<rfid>:<amt>$  (Deposit)
 * #A:BAL:<rfid>$       (Balance Inquiry)
 * #A:PIN:<rfid>:<pin>$  (PIN Change)
 * #A:MST:<rfid>:<txNo>$ (Mini Statement)
 * #A:BLK:<rfid>$       (Block Card)
 * @param head Pointer to the head of the linked list of accounts.
 * @param fd File descriptor for serial communication.
 * @param buf Pointer to the received message buffer containing the action request.
 * @param void No return value.
 */
void act(Acc *head,const int fd,const char *buf){
        //#A:WTD:<rfid>:<amt>$  -> @OK:DONE$,@ERR:LOWBAL$,@ERR:NEGAMT$,@ERR:MAXAMT$ //Withdrawal format and responses
        //#A:DEP:<rfid>:<amt>$  -> @OK:DONE$,@ERR:NEGAMT$,@ERR:MAXAMT$ //Deposit format and responses
        //#A:BAL:<rfid>$        -> @OK:BAL=<amt>$ //Balance inquiry format and response
        //#A:PIN:<rfid>:<pin>$  -> @OK:DONE$ //PIN change format and response
        //#A:MST:<rfid>:<txNo>$ -> @TXN:<type>:<ddmmyyyyhhmm>:<amt>$ //Mini statement format and response
        //#A:BLK:<rfid>$        -> @OK:DONE$ //Block card format and response

        char rfid[9]; //Buffer for extracted RFID
        char req[4]; //Buffer for extracted request code (e.g., "WTD", "DEP") (3 chars + null)
        char pin[5]; //Buffer for extracted new PIN (for PIN change)
        double amt=0; //Variable to store extracted amount for transactions
        char txn=0; //Variable to store transaction number for mini statement

        //extract rfid //Comment indicating RFID extraction
        strncpy(rfid,buf+7,8); //Extracts RFID (8 chars from buf[7], after "#A:XXX:")
        rfid[8]='\0'; //Null-terminates RFID string
        //get Acc //Comment indicating account retrieval
        Acc *usr=getAcc(head,rfid); //Retrieves user account based on RFID
        //extract req //Comment indicating request code extraction
        strncpy(req,buf+3,3); //Extracts the 3-letter request code (from buf[3], after "#A:")
        req[3]='\0'; //Null-terminate the request string (corrected from req[4])

        printf("req=%s,rfid=%s\n",req,rfid); //Debug print of request and RFID

        if(!strcmp(req,"WTD")){ //If request is "WTD" (Withdraw)
                amt=extAmt(buf); //Extracts the withdrawal amount from the buffer
                withdraw(fd,usr,amt); //Calls the withdraw function
                saveData(head); //Saves all account data after the transaction
        }else if(!strcmp(req,"DEP")){ //If request is "DEP" (Deposit)
                amt=extAmt(buf); //Extracts the deposit amount from the buffer
                deposit(fd,usr,amt); //Calls the deposit function
                saveData(head); //Saves all account data
        }else if(!strcmp(req,"BAL")){ //If request is "BAL" (Balance Inquiry)
                balance(fd,usr); //Calls the balance inquiry function
        }else if(!strcmp(req,"MST")){ //If request is "MST" (Mini Statement)
                //mini statement //Comment indicating mini statement logic
                txn=buf[16]-'0'; //Extracts transaction number (single digit char at buf[16]) and converts to int
                miniStatement(fd,usr,txn); //Calls the mini statement function
        }else if(!strcmp(req,"TNF")){ //If request is "TNF" (Transfer - currently not implemented)
            //This block is empty, indicating TNF is a placeholder or future feature
        }else if(!strcmp(req,"PIN")){ //If request is "PIN" (PIN Change)
                strncpy(pin,buf+16,4); //Extracts the new PIN (4 chars from buf[16])
            pin[4] = '\0'; //Null-terminate the pin string
                pinChange(fd,usr,pin); //Calls the PIN change function
                saveData(head); //Saves all account data
        }else if(!strcmp(req,"BLK")){ //If request is "BLK" (Block Card)
                usr->cardStat=BLOCKED; //Sets the user's card status to BLOCKED
#ifdef INT //Conditional compilation for interactive mode
                checkMC(fd); //Performs a microcontroller connectivity check
#endif //End of INT conditional block
                tx_str(fd,"@OK:DONE$"); //Sends a confirmation message
                saveData(head); //Saves all account data
        }else{ //If the request code is unknown
            //This block is empty, unknown requests are ignored
        }
}

/**
 * @brief Extracts a floating-point amount from a message buffer.
 * The amount is assumed to start at index 16 of the buffer and end just before the trailing '$'.
 * Example: #A:WTD:<rfid>:<amt>$ or #A:DEP:<rfid>:<amt>$
 * @param buf Pointer to the character array (message buffer).
 * @return double The extracted amount as a double.
 */
double extAmt(const char *buf){
        char dup[20]; //Temporary buffer to hold the amount string
        strcpy(dup,buf+16); //Copies the substring starting from index 16 (where amount begins) into 'dup'
        dup[strlen(dup)-1]='\0'; //Removes the trailing '$' by replacing it with a null terminator
        return atof(dup); //Converts the amount string 'dup' to a double and returns it
}

/**
 * @brief Searches for an account in the linked list by RFID.
 * @param head Pointer to the head of the linked list of accounts.
 * @param rfid The RFID string to search for.
 * @return Acc* Pointer to the found account structure if successful, NULL otherwise.
 */
Acc* getAcc(Acc *head,const char *rfid){
        while(head){ //Iterates through the linked list of accounts
                if(!strcmp(rfid,head->rfid)) break; //If the current account's RFID matches the search RFID, exit loop
                head=head->nxt; //Moves to the next account in the list
        }
        return head; //Returns the pointer to the found account, or NULL if not found (end of list reached)
}

/// Start of deposit function block marker (custom comment style)
/**
 * @brief Processes a deposit transaction for a user.
 * Validates the amount, updates balance, adds a transaction record, and sends a response.
 * Expected request format leading to this: #A:DEP:<rfid>:<amt>$
 * Responses: @OK:DONE$, @ERR:NEGAMT$, @ERR:MAXAMT$
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account data.
 * @param amt The amount to be deposited.
 * @param void No return value.
 */
void deposit(const int fd,Acc *usr,const f64 amt){
        //#A:DEP:<rfid>:<amt>$  -> @OK:DONE$,@ERR:NEGAMT$,@ERR:MAXAMT$ //Message format and possible responses
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
        if(amt<=0){ //Checks if the deposit amount is non-positive
                tx_str(fd,"@ERR:NEGAMT$"); //Sends "negative amount" error response

        }else if(amt<MAX_DEPOSIT){ //Checks if the amount is within the maximum deposit limit
                usr->bal += amt; //Adds the amount to the user's balance
                //update 2 transc //Comment indicating transaction record update
                addTran(usr,+amt,DEPOSIT); //Adds a new transaction record for this deposit
                tx_str(fd,"@OK:DONE$"); //Sends "DONE" success response

        }else{ //If the amount exceeds the maximum deposit limit
                tx_str(fd,"@ERR:MAXAMT$"); //Sends "maximum amount exceeded" error response

        }
}
/// End of deposit function block marker

/// Start of withdraw function block marker
/**
 * @brief Processes a withdrawal transaction for a user.
 * Validates amount, checks balance, updates balance, adds transaction record, and sends response.
 * Expected request format leading to this: #A:WTD:<rfid>:<amt>$
 * Responses: @OK:DONE$, @ERR:LOWBAL$, @ERR:NEGAMT$, @ERR:MAXAMT$
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account data.
 * @param amt The amount to be withdrawn.
 * @param void No return value.
 */
void withdraw(const int fd,Acc *usr,const f64 amt){
        //#A:WTD:<rfid>:<amt>$  -> @OK:DONE$,@ERR:LOWBAL$,@ERR:NEGAMT$,@ERR:MAXAMT$ //Message format and responses
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
        if(amt<=0){ //Checks if withdrawal amount is non-positive
                tx_str(fd,"@ERR:NEGAMT$"); //Sends "negative amount" error
        }else if(amt<MAX_WITHDRAW){ //Checks if amount is within the maximum withdrawal limit
                if(amt<=(usr->bal)){ //Checks if user has sufficient balance
                        usr->bal -= amt; //Subtracts the amount from user's balance
                        //update 2 transc //Comment indicating transaction record update
                        addTran(usr,-amt,WITHDRAW);//1 //Adds transaction record (amount is negative for withdrawal in history)
                        tx_str(fd,"@OK:DONE$"); //Sends "DONE" success response

                }else{ //If balance is insufficient
                        tx_str(fd,"@ERR:LOWBAL$"); //Sends "low balance" error

                }
        }else{ //If amount exceeds maximum withdrawal limit
                tx_str(fd,"@ERR:MAXAMT$"); //Sends "maximum amount exceeded" error

        }
}
/// End of withdraw function block marker

/**
 * @brief Handles a balance inquiry request for a user.
 * Formats the user's current balance and sends it back via serial.
 * Expected request format leading to this: #A:BAL:<rfid>$
 * Response: @OK:BAL=<balance>$
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account data.
 * @param void No return value.
 */
void balance(const int fd,Acc *usr){
        //#A:BAL:<rfid>$        -> @OK:BAL=<amt>$ //Message format and response
        puts("in bal."); //Debug print to server console
        char buf[50]; //Buffer to format the response string
        sprintf(buf,"@OK:BAL=%.2lf$",usr->bal); //Formats the balance into the response string, to 2 decimal places
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
        tx_str(fd,buf); //Sends the formatted balance response

}
/// End of balance function block marker (custom comment style)

/**
 * @brief Handles a PIN change request for a user.
 * Updates the user's PIN in their account data.
 * Expected request format leading to this: #A:PIN:<rfid>:<newpin>$
 * Response: @OK:DONE$
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account data.
 * @param pin Pointer to the new PIN string (should be 4 characters).
 * @param void No return value.
 */
void pinChange(const int fd,Acc *usr,const char *pin){
        //#A:PIN:<rfid>:<pin>$  -> @OK:DONE$ //Message format and response
        strcpy(usr->pin,pin); //Copies the new PIN into the user's account structure
#ifdef INT //Conditional compilation for interactive mode
        checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
        tx_str(fd,"@OK:DONE$"); //Sends "DONE" success response

}
/// End of pinChange function block marker

/**
 * @brief Retrieves and sends details of a specific transaction (mini statement).
 * Extracts date, time, amount, and type for a given transaction number from user's history.
 * Expected request format: #A:MST:<rfid>:<txNo>$
 * Response: @TXN:<type>:<dd/mm/yyyy hh:mm>:<amt>$ or @TXN:7:0:0$ (if transaction not found or invalid)
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account data.
 * @param txn The transaction number (1-based index) to retrieve from history.
 * @param void No return value.
 */
void miniStatement(const int fd,Acc *usr,char txn){ //txn is char but used as int after '0' subtraction
        //#A:MST:<rfid>:<txNo>$ -> @TXN:<type>:<ddmmyyyyhhmm>:<amt>$ //Message format and response
        char buf[70]; //Buffer to format the transaction detail string for sending
        u64 dum; //Temporary variable for timestamp decomposition
        double amt; //Variable to store transaction amount
        unsigned int dd,mon,yy,hh,mm; //Variables for date and time components
        if(txn > 0 && txn <= usr->tranCnt){ //Checks if the requested transaction number is valid
                Tran *t = usr->tranHist; //Points to the start of the transaction history linked list
                while(--txn){ //Traverses the list to the desired transaction (txn is 1-based)
                        t=t->nxt; //Moves to the next transaction
                }
                dum=(t->id)/100000; //Extracts timestamp part from transaction ID (YYYYMMDDHHMMSSxxx -> YYYYMMDDHHMM)
                mm=dum%100; //Extracts minutes
                dum/=100; //Removes minutes part
                hh=dum%100; //Extracts hours
                dum/=100; //Removes hours part
                dd=dum%100; //Extracts day
                dum/=100; //Removes day part
                mon=dum%100; //Extracts month
                dum/=100; //Removes month part
                yy=dum; //Remaining part is year
                amt=t->amt; //Gets the transaction amount
                amt=(amt<0)?-amt:amt; //Makes the amount positive for display purposes
                sprintf(buf,"@TXN:%d:%02u/%02u/%04u %02u:%02u:%.2lf$",t->type,dd,mon,yy,hh,mm,amt); //Formats the transaction details string
#ifdef INT //Conditional compilation for interactive mode
                checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
                tx_str(fd,buf); //Sends the transaction details
        }else{ //If the transaction number is invalid or out of range
#ifdef INT //Conditional compilation for interactive mode
                checkMC(fd); //Performs microcontroller connectivity check
#endif //End of INT conditional block
                tx_str(fd,"@TXN:7:0:0$"); //Sends an error/placeholder transaction detail (type 7, zero amount/date)
        }
}
/// End of miniStatement function block marker

/**
 * @brief Adds a new transaction record to the beginning of a user's transaction history.
 * Dynamically allocates memory for the new transaction.
 * @param usr Pointer to the user's account structure whose history is to be updated.
 * @param amt The amount of the transaction. Positive for credit (deposit), negative for debit (withdrawal).
 * @param type The type of transaction (e.g., DEPOSIT, WITHDRAW).
 * @param void No return value.
 */
void addTran(Acc *usr,f64 amt,char type){
        Tran *new=calloc(1,sizeof(Tran)); //Allocates and zero-initializes memory for a new transaction node
        new->amt=amt; //Sets the transaction amount
        new->id =getTranId(usr); //Generates and sets a unique transaction ID
        new->type=type; //Sets the transaction type
        new->nxt=NULL; //Initializes the next pointer to NULL

        new->nxt=usr->tranHist; //Links the new transaction to the existing head of the transaction history
        usr->tranHist=new; //Updates the user's transaction history to point to the new transaction as the head

        (usr->tranCnt)++; //Increments the user's transaction counter
}

/**
 * @brief Generates a unique 17-digit transaction ID.
 * The ID is formed by concatenating a 14-digit timestamp (YYYYMMDDHHMMSS)
 * with a 3-digit random number. The random number generator is seeded with the user's account number.
 * @param usr Pointer to the user's account (used to seed srand for pseudo-randomness).
 * @return u64 The generated unique transaction ID.
 */
u64 getTranId(Acc *usr){
        //17 digit unq TranID //Comment describing the transaction ID format
        srand(usr->num); //Seeds the random number generator with the user's account number for varied results per user
        return getTimeStamp()*1000 +(rand()%1000); //Combines timestamp (shifted left by 3 decimal places) and a 0-999 random number
}
/// End of getTranId function block marker

/**
 * @brief Retrieves the current system time and formats it as a 14-digit timestamp.
 * The format is YYYYMMDDHHMMSS.
 * Uses localtime, so it reflects the system's configured timezone.
 * @param void No parameters.
 * @return u64 The current timestamp as an unsigned long long integer.
 */
u64 getTimeStamp(void){
        time_t rawtime; //Variable to store raw time value
        struct tm *timeinfo; //Pointer to structure holding broken-down time components

        // Get current UTC time //Comment states UTC, but localtime() is used below which is typically local time.
        time(&rawtime); //Gets the current calendar time as a time_t object

        // Convert to IST (UTC +5:30) //Comment indicating potential timezone adjustment (currently commented out)
        timeinfo =localtime(&rawtime); //Converts time_t to struct tm in local time

        // Format as<x_bin_102>MMDDHHMMSS (14-digit ID) //Comment describing the output format
        u64 timeStamp = //Calculates the timestamp value
                (timeinfo->tm_year+1900)*10000000000ULL+ //Year (tm_year is years since 1900)
                (timeinfo->tm_mon+1)*100000000ULL+ //Month (tm_mon is 0-11, so +1)
                timeinfo->tm_mday*1000000ULL+ //Day of the month
                timeinfo->tm_hour*10000ULL+ //Hours
                timeinfo->tm_min *100ULL+ //Minutes
                timeinfo->tm_sec; //Seconds

        return timeStamp; //Returns the formatted timestamp
}
// End of getTimeStamp function block marker (custom comment style)
/* //Start of a commented-out block
#ifdef INT
checkMC();
#endif

*/ //End of commented-out block

/**
 * @brief Performs a simple handshake to check connectivity with another device (e.g., MCU).
 * Sends "@Y:LINEOK$" and expects to receive "#Y:LINEOK$".
 * This function is typically called when the INT (interactive) macro is defined.
 * @param fd File descriptor for serial communication.
 * @param void No return value. Loop continues until handshake is successful.
 */
void checkMC(const int fd){
        char buf[20]; //Buffer to receive the response string
        while(1){ //Loop until the correct response is received
                tx_str(fd,"@Y:LINEOK$"); //Sends a line check message
                rx_str(fd,buf,sizeof(buf)); //Receives the response
                if(!strcmp(buf,"#Y:LINEOK$"))break; //If the response matches the expected handshake, exit loop
        }
}
// End of checkMC function block marker

/**
 * @brief Loads account data and transaction histories from CSV files into memory.
 * Reads main account data from "../dataz/Db.csv".
 * For each account, reads its transaction history from "../dataz/<account_number>.csv".
 * Builds a linked list of accounts, each with its linked list of transactions.
 * @param head A pointer to the Acc* pointer that will store the head of the loaded account list.
 * @param void No return value.
 */
void syncData(Acc **head){
        FILE *fp=fopen("../dataz/Db.csv","r"); //Opens the main database file for reading
        int d; //Unused variable
        if(!fp)return; //If the file cannot be opened, return (database remains empty or as is)
        puts("syncing"); //Prints "syncing" to console to indicate data loading process
        Acc temp,*tail=NULL; //temp: temporary Acc structure to read data into, tail: pointer to the last node in the list

        temp.nxt=NULL; //Initializes next pointer of temp (important for memmove)
        temp.tranHist=NULL; //Initializes transaction history of temp
        temp.tranCnt=0; //Initializes transaction count of temp
        //Reads account data line by line from Db.csv
        while(fscanf(fp,"%llu,%[^,],%llu,%[^,],%[^,],%[^,],%[^,],%d,%lf,%llu",
                &(temp.num),temp.name,&(temp.phno),temp.usrName,temp.pass,
                temp.rfid,temp.pin,&(temp.cardStat),&(temp.bal),&(temp.tranCnt))==10){ //Reads 10 fields per account


                Acc *new =calloc(1,sizeof(Acc)); //Allocates memory for a new account node
                memmove(new,&temp,sizeof(Acc)); //Copies the data from 'temp' to the new 'new' node
                new->tranHist = NULL; //Explicitly set tranHist to NULL for the new node before loading its transactions
                new->tranCnt = 0; //Explicitly set tranCnt to 0 for the new node
                if(!(*head))*head=new; //If the list is empty, the new node becomes the head
                if(tail)tail->nxt=new; //If the list is not empty, append the new node to the end
                tail=new; //Update the tail pointer to the new node

                //save bank statement //Comment indicating loading of transaction history (statement)
                char spName[30]; //Buffer for the transaction file name
                sprintf(spName,"../dataz/%llu.csv",new->num); //Formats the transaction file name using account number
                FILE *sp=fopen(spName,"r"); //Opens the account-specific transaction file for reading
                if(!sp)continue; //If transaction file doesn't exist or can't be opened, skip to next account
                Tran *th=NULL,*tt=NULL,tm; //th: head of transaction list for current account, tt: tail, tm: temporary Tran
                int cnt=0; //Counter for transactions loaded for the current account
                tm.nxt=NULL; //Initialize next pointer for tm (important for memmove)
                //Reads transaction data line by line from the account's .csv file
                while(fscanf(sp,"%llu,%lf,%c",&(tm.id),&(tm.amt),&(tm.type))==3){ //Reads 3 fields per transaction
                        Tran *c=malloc(sizeof(Tran)); //Allocates memory for a new transaction node
                        cnt++; //Increments transaction counter for this account
                        memmove(c,&tm,sizeof(Tran)); //Copies data from 'tm' to new transaction node 'c'
                        c->nxt = NULL; // Ensure the new transaction node's next pointer is NULL before linking
                        if(!th)th=c; //If this is the first transaction, it becomes the head (th)
                        if(tt)tt->nxt=c; //Append to the end of the transaction list
                        tt=c; //Update tail (tt) of the transaction list
                }
                new->tranHist=th; //Assigns the loaded transaction history to the current account
                new->tranCnt=cnt; //Assigns the loaded transaction count to the current account
                fclose(sp); //Closes the account-specific transaction file
        }

        fclose(fp); //Closes the main database file (Db.csv)
}

// End of syncData function block marker

/**
 * @brief Saves the current state of all accounts and their transaction histories to CSV files.
 * Writes main account data to "../dataz/Db.csv".
 * For each account, writes its transaction history to "../dataz/<account_number>.csv".
 * This function is for creating machine-readable data backups.
 * @param head Pointer to the head of the linked list of accounts.
 * @param void No return value.
 */
void saveData(Acc *head){

        FILE *fp=fopen("../dataz/Db.csv","w"); //Opens/creates the main database file for writing (overwrites existing)

        while(head){ //Iterates through each account in the linked list
                //Writes account details to Db.csv
                fprintf(fp,"%llu,%s,%llu,%s,%s,%s,%s,%d,%lf,%llu\n",head->num,head->name,head->phno,
                                head->usrName,head->pass,head->rfid,head->pin,head->cardStat,head->bal,head->tranCnt);

                //save bank statement //Comment indicating saving of transaction history
                char spName[30]; //Buffer for transaction file name
                sprintf(spName,"../dataz/%llu.csv",head->num); //Formats the transaction file name
                FILE *sp=fopen(spName,"w"); //Opens/creates the account-specific transaction file for writing
                Tran *t=head->tranHist; //Points to the head of the current account's transaction history
                while(t){ //Iterates through each transaction for the current account
                        fprintf(sp,"%llu,%lf,%c\n",t->id,t->amt,t->type); //Writes transaction details to the file
                        t=t->nxt; //Moves to the next transaction
                }
                fclose(sp); //Closes the account-specific transaction file
                head=head->nxt; //Moves to the next account in the main list
        }
        fclose(fp); //Closes the main database file (Db.csv)
}
// End of saveData function block marker

// End of saveFile function block marker (misplaced, should be above saveFile)
/**
 * @brief Saves the current state of all accounts and their transaction histories
 * to human-readable CSV files in the "../filez/" directory.
 * Writes main account overview to "../filez/DataBase.csv" with headers.
 * For each account, writes its formatted transaction history to "../filez/<account_number>.csv" with headers.
 * @param head Pointer to the head of the linked list of accounts.
 * @param void No return value.
 */
void saveFile(Acc *head){
        unsigned int dd,mon,yy,hh,mm; //Variables for date and time components from transaction ID
        u64 dum; //Temporary variable for timestamp decomposition
        FILE *fp=fopen("../filez/DataBase.csv","w"); //Opens/creates the human-readable main database file

        //Writes headers to the main human-readable database file
        fprintf(fp,"Account ID,Holder's name,Mobile no.,Username,Password,ATM card no.,ATM pin,Card Satus,Balance,Transactions count\n");
        Acc *currentAcc = head; //Use a temporary pointer to iterate, to keep original head if needed later
        while(currentAcc){ //Iterates through each account
                //Writes account details to DataBase.csv, converting card status to "ACTIVE" or "BLOCKED"
                fprintf(fp,"%llu,%s,%llu,%s,%s,%s,%s,%s,%lf,%llu\n",currentAcc->num,currentAcc->name,currentAcc->phno,
                                currentAcc->usrName,currentAcc->pass,currentAcc->rfid,currentAcc->pin,(currentAcc->cardStat)?"ACTIVE":"BLOCKED"
                                ,currentAcc->bal,currentAcc->tranCnt);

                //save bank statement //Comment indicating saving of human-readable transaction history
                char spName[40]; //Buffer for the transaction statement file name
                sprintf(spName,"../filez/%llu.csv",currentAcc->num); //Formats the statement file name
                FILE *sp=fopen(spName,"w"); //Opens/creates the account-specific statement file
                Tran *t=currentAcc->tranHist; //Points to the head of the current account's transaction history
                fprintf(sp,"Date,Time,Transaction ID,Amount,Type\n"); //Writes headers to the statement file
                while(t){ //Iterates through each transaction
                        //Decomposes transaction ID to get date and time
                        dum=(t->id)/100000; //Extracts YYYYMMDDHHMM part
                        mm=dum%100; //Extracts minutes
                        dum/=100;
                        hh=dum%100; //Extracts hours
                        dum/=100;
                        dd=dum%100; //Extracts day
                        dum/=100;
                        mon=dum%100; //Extracts month
                        dum/=100;
                        yy=dum; //Extracts year
                        //Writes formatted transaction details: Date, Time, ID, Amount, Type
                        fprintf(sp,"%u/%u/%u,%u:%u,%llu,%lf,",dd,mon,yy,hh,mm,t->id,t->amt);
                        if(t->type==DEPOSIT)            fprintf(sp,"%s\n","Deposit"); //Prints "Deposit" if type is DEPOSIT
                        else if(t->type==WITHDRAW)      fprintf(sp,"%s\n","Withdraw"); //Prints "Withdraw" if type is WITHDRAW
                        else if(t->type==TRANSFER_IN)   fprintf(sp,"%s\n","Tranfer IN"); //Prints "Transfer IN" (Note: "Tranfer" typo)
                        else if(t->type==TRANSFER_OUT)  fprintf(sp,"%s\n","Tranfer OUT"); //Prints "Transfer OUT" (Note: "Tranfer" typo)
                        t=t->nxt; //Moves to the next transaction
                }
                fclose(sp); //Closes the account-specific statement file
                currentAcc=currentAcc->nxt; //Moves to the next account in the main list
        }
        fclose(fp); //Closes the main human-readable database file
}
