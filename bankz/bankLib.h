// bankers header file
// This file defines the structures, constants, and function prototypes
// used throughout the banking application. It serves as the public interface
// for the banking library.
//

#ifndef _BANKLIB_H_ // Inclusion guard to prevent multiple inclusions of this header.
#define _BANKLIB_H_ // Defines the macro _BANKLIB_H_ if not already defined.


#define MAX_DEPOSIT      30000 //30K // Maximum amount allowed for a single deposit transaction.
#define MAX_WITHDRAW     30000 //30k // Maximum amount allowed for a single withdrawal transaction.
#define MAX_TRANSFER     100000//1lk // Maximum amount allowed for a single transfer transaction.

#define NAME_LEN 30 // Maximum length for names (e.g., account holder name, buffer for general strings).
#define MAX_PASS_LEN 20 // Maximum length for user passwords.
#define MAX_USRN_LEN 20 // Maximum length for usernames.

#define ADMIN_USRN "admin" // Predefined username for the administrator.
#define ADMIN_PASS "admin" // Predefined password for the administrator.
#define ADMIN_EXIT "exit"  // Special password string for admin to exit the application.

#define WITHDRAW 1       // Constant representing a withdrawal transaction type.
#define DEPOSIT  2       // Constant representing a deposit transaction type.
#define TRANSFER_IN 3    // Constant representing a transfer-in transaction type.
#define TRANSFER_OUT 4   // Constant representing a transfer-out transaction type.

#define CAPS(ch) (ch &=~(32)) // Macro to convert a lowercase character to uppercase by clearing the 6th bit.

//decorations - ANSI escape codes for text coloring in the console.
#define RESET   "\033[0m"      // Resets text color to default.
#define BBLACK  "\033[1;30m"    // Bold Black text.
#define BRED    "\033[1;31m"    // Bold Red text.
#define BGREEN  "\033[1;32m"    // Bold Green text.
#define BYELLOW "\033[1;33m"    // Bold Yellow text.
#define BBLUE   "\033[1;34m"    // Bold Blue text.
#define BPINK   "\033[1;35m"    // Bold Pink text.
#define BCYAN   "\033[1;36m"    // Bold Cyan text.
#define BWHITE  "\033[1;37m"    // Bold White text.

typedef unsigned long long int u64; // Typedef for unsigned 64-bit integer, commonly used for IDs and large numbers.
typedef double f64;                 // Typedef for double-precision floating-point number, used for monetary amounts.

// Structure to represent a single transaction.
// Used to store details of each financial operation like deposit, withdrawal, or transfer.
// Format for storage/parsing: "%lu,%lf,%c",id,amt,type
typedef struct A{
        f64 amt;          // The amount of money involved in the transaction. Positive for deposit/transfer_in, negative for withdrawal/transfer_out.
        u64 id;           // Unique identifier for this transaction.
        char type;        // Type of the transaction (e.g., WITHDRAW, DEPOSIT, TRANSFER_IN, TRANSFER_OUT).

        struct A *nxt;    // Pointer to the next transaction in a linked list (for transaction history).
}Tran;

// Structure to represent a bank account.
// Contains all details related to a customer's account.
// Format for storage/parsing: "%lu,%s,%lu,%s,%s,%s,%s,%d,%lf,%lu",num,name,phno,usrName,pass,rfid,pin,cardStat,bal,tranCnt
typedef struct B{
        u64 num;                    // Unique account number/identifier for this account.
        f64 bal;                    // Current bank balance of the account.
        u64 phno;                   // Phone number of the account holder.
        char usrName[MAX_USRN_LEN]; // Username for logging into the account.
        char pass[MAX_PASS_LEN];    // Password for logging into the account.
        char rfid[9];               // RFID card number associated with the account (8 digits + null terminator).
        char pin[5];                // ATM PIN for the card (4 digits + null terminator).
        int cardStat;               // Status of the ATM card: 1-active, 0-blocked due to wrong login, 2-blocked during pin change.
        char *name;                 // Name of the account holder (dynamically allocated).

        Tran *tranHist;             // Pointer to the head of a linked list of transactions (transaction history).
        u64 tranCnt;                // Total count of transactions for this account.
        struct B *nxt;              // Pointer to the next account in a linked list (for the database of accounts).
}Acc;

extern const int szDb; // Declaration of a global constant, likely representing a size related to database entries.

//**functions**\\

//Account-handling
//main

/**
 * @brief Displays the initial login menu.
 * This function prints the welcome message and prompts for login credentials.
 */
void loginMenu(void);

/**
 * @brief Displays the menu for administrator actions.
 * This function lists the operations available to an authenticated administrator.
 */
void adminMenu(void);

/**
 * @brief Displays the menu for customer actions.
 * This function lists the operations available to an authenticated customer.
 */
void userMenu(void);

/**
 * @brief Displays the menu for account update options.
 * This function lists the fields that can be updated for an account.
 */
void accMenu(void);

/**
 * @brief Displays a general menu (purpose might be similar to admin or a combined menu).
 * Note: The specific use case for this menu versus adminMenu/userMenu should be clarified by its usage in bank_main.c.
 */
void menu(void); // This seems to be a general menu, its exact context of use should be checked.

/**
 * @brief Validates user credentials against the account database.
 * @param head Pointer to the head of the accounts linked list.
 * @param usr The username to validate.
 * @param pass The password to validate.
 * @return Pointer to the matching Acc structure if credentials are valid, NULL otherwise.
 */
Acc* isValid(Acc*,char*,char*);

/**
 * @brief Checks if a username is unique (not already in use and not admin's username).
 * @param head Pointer to the head of the accounts linked list.
 * @param usr The username to check for uniqueness.
 * @return 1 if the username is unique, 0 otherwise.
 */
int isUnq(Acc*,char*);

/**
 * @brief Checks if an RFID card number is new (not already associated with another account).
 * @param head Pointer to the head of the accounts linked list.
 * @param rf The RFID string to check.
 * @return 1 if the RFID is new, 0 otherwise.
 */
int isNewRFID(Acc *head,char *rf);

/**
 * @brief Displays the details of a specific account.
 * @param usr Pointer to the Acc structure whose details are to be displayed.
 */
void dispAcc(Acc*);

/**
 * @brief Creates a new bank account and adds it to the database.
 * Prompts the user for all necessary details to set up a new account.
 * @param head Double pointer to the head of the accounts linked list, allowing modification of the list head.
 */
void newAcc(Acc**);

/**
 * @brief Updates information for an existing bank account.
 * @param head Double pointer to the head of the accounts linked list (used for username uniqueness checks).
 * @param usr Pointer to the Acc structure of the account to be updated.
 */
void updateAcc(Acc**,Acc*);

/**
 * @brief Deletes an existing bank account from the database.
 * (Note: Implementation might be missing or incomplete based on bankLib.c).
 * @param head Double pointer to the head of the accounts linked list, allowing modification of the list.
 */
void dltAcc(Acc**);

/**
 * @brief Retrieves a specific account from the database based on search criteria.
 * Prompts the user for how they want to search (e.g., by phone number, account number).
 * @param head Pointer to the head of the accounts linked list.
 * @return Pointer to the found Acc structure, or NULL if not found or invalid choice.
 */
Acc* getAcc(Acc*);

/**
 * @brief Performs a withdrawal operation for the specified account.
 * Prompts for the amount and validates against balance and withdrawal limits.
 * @param usr Pointer to the Acc structure for the account.
 */
void withdraw(Acc*);

/**
 * @brief Performs a deposit operation for the specified account.
 * Prompts for the amount and validates against deposit limits.
 * @param usr Pointer to the Acc structure for the account.
 */
void deposit(Acc*);

/**
 * @brief Transfers money from one account to another.
 * Prompts for the amount and validates against sender's balance and transfer limits.
 * @param from Pointer to the sender's Acc structure.
 * @param to Pointer to the receiver's Acc structure.
 */
void transfer(Acc*,Acc*);

/**
 * @brief Adds a new transaction record to an account's transaction history.
 * @param usr Pointer to the Acc structure to which the transaction belongs.
 * @param amt The amount of the transaction.
 * @param type The type of the transaction (WITHDRAW, DEPOSIT, etc.).
 */
void addTran(Acc*,f64,char);

/**
 * @brief Displays the current balance of the specified account.
 * @param usr Pointer to the Acc structure.
 */
void balance(Acc*);

/**
 * @brief Displays the transaction history for the specified account.
 * @param usr Pointer to the Acc structure.
 */
void statement(Acc*);

/**
 * @brief Displays a summary of all accounts in the database.
 * Shows Account ID, Holder Name, Mobile, and Transaction Count for each account.
 * @param head Pointer to the head of the accounts linked list.
 */
void database(Acc*);

///sub - Utility or helper functions

/**
 * @brief Generates a unique account ID.
 * The ID is typically based on a timestamp and a random number to ensure uniqueness.
 * @param head Pointer to the head of the accounts linked list (to check for collisions).
 * @return A unique u64 account ID.
 */
u64    getUnqId(Acc*);

/**
 * @brief Generates a unique transaction ID for an account.
 * The ID is typically based on a timestamp and a random number, possibly seeded with account info.
 * @param usr Pointer to the Acc structure for which the transaction ID is generated.
 * @return A unique u64 transaction ID.
 */
u64    getTranId(Acc*);

/**
 * @brief Gets the current timestamp formatted as a u64 integer (YYYYMMDDHHMMSS).
 * @return The current timestamp as a u64 value.
 */
u64    getTimeStamp(void);

/**
 * @brief Gets a single character input from the user, typically for menu choices.
 * Converts the character to uppercase.
 * @return The uppercase character entered by the user.
 */
char   getKey(void);

/**
 * @brief Reads a string from standard input.
 * Allocates memory for the string dynamically. The caller is responsible for freeing this memory.
 * @return A pointer to the dynamically allocated string.
 */
char* getStr(void);

/**
 * @brief Formats a string, typically by capitalizing the first letter of each word.
 * @param str Pointer to the string to be formatted in place.
 */
void  format(char*);

//file-handling

/**
 * @brief Saves the current state of all accounts and their transaction histories to data files.
 * Typically creates/overwrites CSV files in a 'dataz' directory.
 * @param head Pointer to the head of the accounts linked list.
 */
void saveData(Acc *);

/**
 * @brief Saves account data and transaction histories to human-readable files.
 * Typically creates/overwrites CSV files in a 'filez' directory, with more formatted output.
 * @param head Pointer to the head of the accounts linked list.
 */
void saveFile(Acc *head);

/**
 * @brief Loads account data and transaction histories from data files into memory at startup.
 * Reads from CSV files in the 'dataz' directory to reconstruct the linked list of accounts.
 * @param head Double pointer to the head of the accounts linked list, to populate the list.
 */
void syncData(Acc **);

#endif // End of inclusion guard for _BANKLIB_H_
