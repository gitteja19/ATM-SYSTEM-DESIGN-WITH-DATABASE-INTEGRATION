#ifndef _ATMLIB_H //If _ATMLIB_H is not defined
#define _ATMLIB_H //Define _ATMLIB_H to prevent multiple inclusions of this header file

/*
 * atmLib.h
 *
 * Header file for the ATM Simulation Project.
 * This file contains:
 * - Macro definitions for constants and debugging.
 * - Standard library includes.
 * - Type definitions for custom data structures (u64, f64, Tran, Acc).
 * - Function prototypes for ATM operations, serial communication,
 * data handling, and utility functions.
 */

#define DBG //Macro to enable debug messages/mode
//#define INT //Macro to enable interactive mode features (currently commented out)

#include<stdio.h> //Standard Input/Output operations
#include<stdlib.h> //Standard library functions (like malloc, calloc, exit)
#include<stdint.h> //Standard integer types
#include<string.h> //String manipulation functions
#include<unistd.h> //POSIX operating system API (e.g., read, write, close, usleep)
#include<fcntl.h> //File control options (e.g., open, fcntl)
#include<errno.h> //Error number definitions
#include<time.h> //Time and date functions
#include<termios.h> //POSIX terminal control definitions (for serial communication)

#define MAX_DEPOSIT     30000 //Defines the maximum deposit amount allowed in a single transaction (30K)
#define MAX_WITHDRAW    30000 //Defines the maximum withdrawal amount allowed in a single transaction (30k)
#define MAX_TRANSFER    100000//Defines the maximum transfer amount allowed in a single transaction (1lk)

#define NAME_LEN 30 //Defines the maximum length for account holder names
#define MAX_PASS_LEN 20 //Defines the maximum length for passwords
#define MAX_USRN_LEN 20 //Defines the maximum length for usernames

#define BLOCKED 0 //Defines the status code for a blocked card
#define ACTIVE  1 //Defines the status code for an active card

#define WITHDRAW 1 //Defines the transaction type code for withdrawal
#define DEPOSIT  2 //Defines the transaction type code for deposit
#define TRANSFER_IN 3 //Defines the transaction type code for transfer in
#define TRANSFER_OUT 4 //Defines the transaction type code for transfer out

#define CAPS(ch) (ch &=~(32)) //Macro to convert a character to uppercase (by clearing the 6th bit)

//decorations //ANSI escape codes for text color formatting in the console
#define RESET   "\033[0m" //Resets text formatting to default
#define BBLACK  "\033[1;30m" //Bold Black text
#define BRED    "\033[1;31m" //Bold Red text
#define BGREEN  "\033[1;32m" //Bold Green text
#define BYELLOW "\033[1;33m" //Bold Yellow text
#define BBLUE   "\033[1;34m" //Bold Blue text
#define BPINK   "\033[1;35m" //Bold Pink text
#define BCYAN   "\033[1;36m" //Bold Cyan text
#define BWHITE  "\033[1;37m" //Bold White text


typedef unsigned long long int u64; //Typedef for unsigned 64-bit integer
typedef double f64; //Typedef for double-precision floating-point number (64-bit)

// "%lu,%lf,%c",id,amt,type //Format string comment for transaction data in files
// "%lu,%s,%lu,%s,%s,%s,%s,%d,%lf,%lu",num,name,phno,usrName,pass,rfid,pin,cardStat,bal,tranCnt //Format string comment for account data in files
typedef struct A{ //Structure to represent a single transaction
        f64 amt; //Amount of the transaction
        u64 id; //Unique ID for the transaction
        char type; //Type of transaction (e.g., WITHDRAW, DEPOSIT)

        struct A *nxt; //Pointer to the next transaction in a linked list (for transaction history)
}Tran; //Typedef name for struct A

// "%lu,%s,%lu,%s,%s,%s,%s,%d,%lf,%lu",num,name,phno,usrName,pass,rfid,pin,cardStat,bal,tranCnt //Format string comment for account data
typedef struct B{ //Structure to represent a bank account
        u64 num;//Unique account number/ID
        f64 bal;//Current bank balance of the account
        u64 phno;//Phone number of the account holder
        char usrName[MAX_USRN_LEN]; //Username associated with the account
        char pass[MAX_PASS_LEN]; //Password for the account
        char rfid[9]; //RFID card number (8 chars + null terminator)
        char pin[5]; //ATM PIN (4 digits + null terminator)
        int cardStat;//Card status: 1 for active, 0 for blocked
        char name[NAME_LEN];//Name of the account holder

        Tran *tranHist; //Pointer to the head of the linked list of transactions for this account
        u64 tranCnt; //Total count of transactions for this account
        struct B *nxt; //Pointer to the next account in a linked list (for the database)
}Acc; //Typedef name for struct B


//Function Prototypes

/**
 * @brief Initializes the serial port for communication.
 * @return int File descriptor for the opened serial port, or -1 on error.
 */
int initSerial();

/**
 * @brief Closes the serial port.
 * @param fd File descriptor of the serial port to close.
 */
void endSerial(const int fd);

/**
 * @brief Transmits a single character over the serial port.
 * @param fd File descriptor of the serial port.
 * @param ch The character to transmit.
 * @return int Number of bytes written, or -1 on error.
 */
int tx_char(const int fd,const char ch);

/**
 * @brief Transmits a null-terminated string over the serial port, followed by CR LF.
 * @param fd File descriptor of the serial port.
 * @param str The string to transmit.
 * @return int Total number of bytes written (including CR LF), or error status from write.
 */
int tx_str(const int fd,const char *str);

/**
 * @brief Receives a single character from the serial port.
 * @param fd File descriptor of the serial port.
 * @return char The received character, or -1 on error or if no data read.
 */
char rx_char(const int fd);

/**
 * @brief Receives a string from the serial port until a newline or max length is reached.
 * @param fd File descriptor of the serial port.
 * @param str Buffer to store the received string.
 * @param len Maximum length of the buffer.
 */
void rx_str(const int fd,char *str,size_t len);

/**
 * @brief Checks if a received message string is in the expected format (#<data>$).
 * @param buf The message buffer to check.
 * @return int 1 if the message format is OK, 0 otherwise.
 */
int isMsgOk(const char *buf);

/**
 * @brief Checks the provided RFID against the database.
 * Sends response back via serial: "@OK:ACTIVE:<username>$" or "@ERR:BLOCK$" or "@ERR:INVALID$".
 * @param head Pointer to the head of the account database (linked list).
 * @param fd File descriptor for serial communication.
 * @param buf The received message buffer containing the RFID (e.g., "#C:<rfid>$").
 */
void checkRFID(Acc *head,const int fd,const char *buf);

/**
 * @brief Verifies the provided PIN for a given RFID.
 * Sends response back via serial: "@OK:MATCHED$" or "@ERR:WRONG$".
 * @param head Pointer to the head of the account database.
 * @param fd File descriptor for serial communication.
 * @param buf The received message buffer containing RFID and PIN (e.g., "#V:<rfid>:<pin>$").
 */
void verifyPin(Acc *head,const int fd,const char *buf);

/**
 * @brief Performs an ATM action based on the request in the buffer.
 * Actions include withdraw, deposit, balance inquiry, PIN change, mini statement, block card.
 * @param head Pointer to the head of the account database.
 * @param fd File descriptor for serial communication.
 * @param buf The received message buffer containing the action request.
 */
void act(Acc *head,const int fd,const char *buf);

/**
 * @brief Extracts the amount from a message buffer.
 * Assumes amount starts at a specific offset (index 16) in the buffer.
 * @param buf The message buffer (e.g., "#A:WTD:<rfid>:<amt>$" or "#A:DEP:<rfid>:<amt>$").
 * @return double The extracted amount.
 */
double extAmt(const char *buf);

/**
 * @brief Handles a deposit transaction for a user.
 * Sends response back via serial: "@OK:DONE$", "@ERR:NEGAMT$", or "@ERR:MAXAMT$".
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account structure.
 * @param amt The amount to deposit.
 */
void deposit(const int fd,Acc *usr,const f64 amt);

/**
 * @brief Handles a withdrawal transaction for a user.
 * Sends response back via serial: "@OK:DONE$", "@ERR:LOWBAL$", "@ERR:NEGAMT$", or "@ERR:MAXAMT$".
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account structure.
 * @param amt The amount to withdraw.
 */
void withdraw(const int fd,Acc *usr,const f64 amt);

/**
 * @brief Handles a balance inquiry for a user.
 * Sends response back via serial: "@OK:BAL=<balance>$".
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account structure.
 */
void balance(const int fd,Acc *usr);

/**
 * @brief Handles a PIN change request for a user.
 * Sends response back via serial: "@OK:DONE$".
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account structure.
 * @param pin The new PIN to set.
 */
void pinChange(const int fd,Acc *usr,const char *pin);

/**
 * @brief Provides a mini statement (details of a specific transaction).
 * Sends response back via serial: "@TXN:<type>:<ddmmyyyyhhmm>:<amt>$" or "@TXN:7:0:0$" if not found.
 * @param fd File descriptor for serial communication.
 * @param usr Pointer to the user's account structure.
 * @param txn The transaction number (1-based index) to retrieve from history.
 */
void miniStatement(const int fd,Acc *usr,char txn);

/**
 * @brief Adds a new transaction to the user's transaction history.
 * @param usr Pointer to the user's account structure.
 * @param amt The amount of the transaction (positive for deposit/credit, negative for withdrawal/debit).
 * @param type The type of the transaction (e.g., DEPOSIT, WITHDRAW).
 */
void addTran(Acc *usr,f64 amt,char type);

/**
 * @brief Generates a unique transaction ID.
 * Combines a timestamp with a random number.
 * @param usr Pointer to the user's account structure (used to seed srand).
 * @return u64 The generated 17-digit transaction ID.
 */
u64 getTranId(Acc *usr);

/**
 * @brief Gets the current timestamp as a u64 value.
 * Format: YYYYMMDDHHMMSS.
 * @return u64 The current timestamp.
 */
u64 getTimeStamp(void);

/**
 * @brief Checks microcontroller (MCU) connectivity by performing a handshake.
 * Sends "@Y:LINEOK$" and expects "#Y:LINEOK$" back. Used if INT macro is defined.
 * @param fd File descriptor for serial communication.
 */
void checkMC(const int fd);

/**
 * @brief Synchronizes (loads) account data from "Db.csv" and individual transaction files.
 * Populates the linked list of accounts pointed to by head.
 * @param head Pointer to the pointer of the head of the account database.
 */
void syncData(Acc **head);

/**
 * @brief Saves all account data and their transaction histories to "Db.csv" and individual <acc_num>.csv files.
 * This is the primary data saving function for machine readability.
 * @param head Pointer to the head of the account database.
 */
void saveData(Acc *head);

/**
 * @brief Saves all account data and their transaction histories to human-readable CSV files.
 * Main database to "DataBase.csv" and individual statements to "<acc_num>.csv" in "filez" directory.
 * @param head Pointer to the head of the account database.
 */
void saveFile(Acc *head);

/**
 * @brief Retrieves an account from the database using the RFID.
 * @param head Pointer to the head of the account database.
 * @param rfid The RFID string to search for.
 * @return Acc* Pointer to the found account structure, or NULL if not found.
 */
Acc* getAcc(Acc *head,const char *rfid);



#endif //End of _ATMLIB_H guard
