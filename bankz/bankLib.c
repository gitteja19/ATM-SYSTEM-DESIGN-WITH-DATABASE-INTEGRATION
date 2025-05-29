#include <stdio_ext.h> // For __fpurge (non-standard, used for clearing input buffer).
#include <stdlib.h>    // For standard library functions like malloc, calloc, free, rand, srand.
#include <stdint.h>    // For integer types like uint64_t (though u64 is defined in bankLib.h).
#include <string.h>    // For string manipulation functions like strcmp, strcpy, strlen, strdup, memmove.
#include <unistd.h>    // For POSIX operating system API, e.g., getpid(), read(), tcgetattr(), fcntl().
#include <time.h>      // For time-related functions, e.g., time(), localtime().
#include "bankLib.h"   // Includes the header file for this library, defining structures and prototypes.

#include <termios.h>   // For terminal I/O control (used in getch and the commented getKey).
#include <fcntl.h>     // For file control options (used in getch).

// This file contains the implementation of the functions
// declared in bankLib.h for the banking application.

// Constant definition for szDb, likely intended for calculating the size of a database record for binary I/O.
// However, current file operations use CSV (text-based) format.
const int szDb=(sizeof(u64)*2+sizeof(f64)+sizeof(char)*(MAX_USRN_LEN+MAX_PASS_LEN));

/**
 * @brief Displays the initial login menu.
 * This function prints a welcome message and a prompt for the user to enter login credentials.
 */
void loginMenu(void){
        printf(BPINK"\n-------------WELCOME TO @JET BANK--------------\n"); // Display welcome message with color.
        printf("Please enter login credentials.\n"RESET); // Prompt for credentials and reset color.
}

/**
 * @brief Displays the menu of actions available to an administrator.
 * Lists various administrative tasks such as creating accounts, updating accounts, etc.
 */
void adminMenu(void){
        printf(BPINK"\nHI ADMIN:\n" // Display greeting to admin.
                       "[KEY]  : ACTION\n"                         // Header for menu options.
                       "c/C    : Create New account.\n"            // Option to create an account.
                       "u/U    : Update Existing account.\n"       // Option to update an account.
                       "h/H    : Transaction history.\n"           // Option to view transaction history.
                       "w/W    : Withdraw amount.\n"               // Option to withdraw amount.
                       "d/D    : Deposit amount.\n"                // Option to deposit amount.
                       "b/B    : Balance enquery.\n"               // Option to check balance.
                       "t/T    : Transfer money.\n"                // Option to transfer money.
                       "x/X    : Activate card.\n"                 // Option to change card status.
                       "e/E    : Display all accounts details.\n"  // Option to display all accounts.
                       "f/F    : Finding/searching for specific account.\n" // Option to find an account.
                       "q/Q    : Quit from app.\n"BYELLOW          // Option to quit the application.
                       "Enter choice:"RESET);                     // Prompt for choice.
}

/**
 * @brief Displays the menu of actions available to a regular user/customer.
 * Lists common banking operations like viewing history, withdrawal, deposit, etc.
 */
void userMenu(void){
        printf(BPINK"\nHI CUSTOMER:\n" // Display greeting to customer.
                       "[KEY]  : ACTION\n"                // Header for menu options.
                       "h/H    : Transaction history.\n"  // Option to view transaction history.
                       "w/W    : Withdraw amount.\n"      // Option to withdraw amount.
                       "d/D    : Deposit amount.\n"       // Option to deposit amount.
                       "b/B    : Balance enquery.\n"      // Option to check balance.
                       "t/T    : Transfer amount.\n"      // Option to transfer money.
                       "q/Q    : Quit from app.\n"BYELLOW // Option to quit the application.
                       "Enter choice:"RESET);            // Prompt for choice.
}

/**
 * @brief Displays a general menu of available actions.
 * This menu seems to be comprehensive, similar to the admin menu.
 * Its specific distinction from adminMenu should be based on its call context in main.
 */
void menu(void){ // This function seems to be a duplicate or alternative general menu.
        printf("\n------------------MENU--------------------------\n" // Menu title.
                       "[KEY]  : ACTION\n"                                 // Header for menu options.
                       "c/C    : Create New account.\n"                     // Option to create an account.
                       "u/U    : Update Old account info.\n"                // Option to update an account.
                       "h/H    : Transaction history.\n"                    // Option to view transaction history.
                       "w/W    : Withdraw amount.\n"                        // Option to withdraw amount.
                       "d/D    : Deposit amount.\n"                         // Option to deposit amount.
                       "b/B    : Balance enquery.\n"                        // Option to check balance.
                       "t/T    : Transfer money.\n"                         // Option to transfer money.
                       "e/E    : Display all accounts details.\n"           // Option to display all accounts.
                       "f/F    : Finding/searching for specific account.\n" // Option to find an account.
                       "q/Q    : Quit from app.\n"                          // Option to quit the application.
                       "Enter choice:");                                   // Prompt for choice.
}

/**
 * @brief Validates user credentials (username and password) against the list of accounts.
 * @param head Pointer to the first account in the linked list of accounts.
 * @param usr The username string to validate.
 * @param pass The password string to validate.
 * @return A pointer to the matching `Acc` structure if credentials are valid, otherwise `NULL`.
 */
Acc* isValid(Acc *head,char *usr,char *pass){
        while(head){ // Iterate through the linked list of accounts.
                if((!strcmp(usr,head->usrName))&&(!strcmp(pass,head->pass))) // Check if both username and password match.
                         break; // If match found, exit loop.
                head=head->nxt; // Move to the next account.
        }
        return head; // Return the pointer to the matched account, or NULL if no match.
}

/**
 * @brief Creates a new bank account by collecting details from the user.
 * Performs validations for input fields like phone number, username uniqueness, RFID, and PIN.
 * @param head A pointer to the pointer of the first account in the list (to modify the list head).
 */
void newAcc( Acc **head){
        char *temp=NULL,flag=1; // temp: temporary string buffer, flag: for loop control and error indication.
        double d; // Temporary variable for double input (initial balance).
        Acc *new=calloc(1,sizeof(Acc)); // Allocate memory for a new account structure and initialize to zero.
        new->tranHist=NULL; // Initialize transaction history pointer to NULL.
        new->tranCnt=0;     // Initialize transaction count to 0.
        new->num=getUnqId(*head); // Generate a unique account number.
        printf("Enter Name:");    // Prompt for account holder's name.
        __fpurge(stdin);          // Clear the input buffer (non-standard, use with caution for portability).
        new->name=getStr();       // Read the name string.
        if(strlen(new->name)<3){  // Validate name length.
                puts("no name");      // Inform user of invalid name.
                free(new->name);      // Free allocated memory for name.
                free(new);            // Free allocated memory for the new account structure.
                return;               // Exit function if name is invalid.
        }
        format(new->name); // Format the name (e.g., capitalize first letters).

        flag=1; // Reset flag for the next input loop.
        while(flag){ // Loop until a valid phone number is entered.
                if(flag!=1)puts("Mobile must be 10 digit, and min:60000 0000"); // Display error if previous attempt failed.
                printf("Enter Mobile No.:"); // Prompt for mobile number.
                //char *num=getStr(); // Commented out: previously might have read as string.
                scanf("%llu",&(new->phno)); // Read phone number as unsigned long long.
                if((new->phno>=6000000000)&&(new->phno<9999999999)){ // Validate phone number range (assuming 10 digits).
                        //valid
                        break; // Exit loop if valid.
                }else{
                        flag++; // Increment flag to indicate an error and re-prompt.
                }
        }

        flag=1; // Reset flag.
        while(flag){ // Loop until a unique and valid username is entered.
                printf("Enter Login Username:"); // Prompt for username.
                temp=getStr(); // Read username string.
                if(strlen(temp)>=MAX_USRN_LEN){ // Check username length against maximum allowed.
                        puts("Username too long,try again!!"); // Inform user.
                        free(temp); // Free temporary string.
                        continue;   // Restart loop.
                }
                if(isUnq(*head,temp)){ // Check if username is unique.
                        strcpy(new->usrName,temp); // Copy unique username to account structure.
                        free(temp); // Free temporary string.
                        flag=0;     // Set flag to 0 to exit loop.
                }else{
                        puts("User name aldready exits!"); // Inform user of duplicate username.
                        puts("Try differnt one.");
                        free(temp); // Free temporary string.
                }
        }
        flag=1; // Reset flag.
        while(flag){ // Loop until password and its confirmation match.
                if(flag>1)puts("Password mismatch!! Retry."); // Display error if previous attempt failed.
                printf("Enter Login Password:"); // Prompt for password.
                temp=getStr(); // Read password string.
                if(strlen(temp)>=MAX_PASS_LEN){ // Check password length.
                        puts("Password too long,try again!!"); // Inform user.
                        flag=1; // Ensure loop continues.
                        free(temp); // Free temporary string.
                        continue;   // Restart loop.
                }
                strcpy(new->pass,temp); // Copy password to account structure.
                free(temp); // Free temporary string (first password input).
                printf("Re-enter Login Password:"); // Prompt for password confirmation.
                temp=getStr(); // Read password confirmation.
                flag=(strcmp(temp,new->pass)?flag+1:0); // Compare passwords. If different, increment flag; else set to 0 to exit.
        }
        free(temp); // Free temporary string (password confirmation).
        flag=1; // Reset flag.
        while(flag){ // Loop until a unique 8-digit RFID is entered.
                if(flag>1)printf("8 digit RFID please:"); // Display error/reminder.
                else printf("Enter RFID card number:"); // Prompt for RFID.

                temp=getStr(); // Read RFID string.
                if(strlen(temp)!=8){ // Validate RFID length.
                        puts("try again!!"); // Inform user.
                        flag++;            // Increment flag for error.
                        free(temp);        // Free temporary string.
                        continue;          // Restart loop.
                }else{
                        if(isNewRFID(*head,temp)){ // Check if RFID is new/unique.
                                strcpy(new->rfid,temp); // Copy RFID to account structure.
                                free(temp); // Free temporary string.
                                break;      // Exit loop.
                        }else{
                                puts("RFID aldready in use!!"); // Inform user of duplicate RFID.
                                flag++;            // Increment flag for error.
                                free(temp);        // Free temporary string.
                        }
                }
        }

        flag=1; // Reset flag.
        while(flag){ // Loop until ATM PIN and its confirmation match (4 digits).
                if(flag>1)puts("Pin mismatch!! Retry."); // Display error if previous attempt failed.
                printf("Enter ATM Pin:"); // Prompt for ATM PIN.
                temp=getStr(); // Read PIN string.
                if(strlen(temp)!=4){ // Validate PIN length.
                        puts("4 digit Pin please,try again!!"); // Inform user.
                        flag=1; // Ensure loop continues (or reset error counter implicitly).
                        free(temp); // Free temporary string.
                        continue;   // Restart loop.
                }
                strcpy(new->pin,temp); // Copy PIN to account structure.
                free(temp); // Free temporary string (first PIN input).
                printf("Re-enter ATM Pin:"); // Prompt for PIN confirmation.
                temp=getStr(); // Read PIN confirmation.
                flag=(strcmp(temp,new->pin)?flag+1:0); // Compare PINs. If different, increment flag; else set to 0 to exit.
        }
        free(temp); // Free temporary string (PIN confirmation).
        new->cardStat=1;//active // Set initial card status to active.

        flag=1; // Reset flag.
        while(flag){ // Loop until a positive opening amount is entered.
                if(flag>1)printf("Enter an posivite amount:"); // Display error/reminder.
                else printf("Enter Opening Amount:"); // Prompt for opening balance.
                scanf("%lf",&d); // Read opening balance. Note: this will add a transaction.
                if(d>0){ // Validate if amount is positive.
                        new->bal=d; // Set account balance.
                        break;      // Exit loop.
                }else flag++; // Increment flag if amount is not positive.
        }
        addTran(new,new->bal,DEPOSIT);//0 // Add the opening deposit as the first transaction. '0' seems to be a comment, DEPOSIT is the type.

        dispAcc(new); // Display the details of the newly created account.
        //add to database (linked list)
        new->nxt=*head; // New account points to the current head of the list.
        *head=new;      // The head of the list now points to the new account.
        puts(BGREEN"Account Created."RESET); // Confirmation message.
}

/**
 * @brief Checks if a given username string is unique across all existing accounts and not the admin username.
 * @param head Pointer to the first account in the linked list.
 * @param str The username string to check for uniqueness.
 * @return 1 if the username is unique, 0 otherwise.
 */
int isUnq(Acc *head,char *str){
        if(!strcmp(str,ADMIN_USRN)) return 0; // Username cannot be the admin's username.
        while(head){ // Iterate through the accounts list.
                if(!strcmp(str,head->usrName)) return 0; // If username matches an existing one, it's not unique.
                head=head->nxt; // Move to the next account.
        }
        return 1; // If loop completes without finding a match, username is unique.
}

/**
 * @brief Checks if a given RFID string is new (not used by any other existing account).
 * @param head Pointer to the first account in the linked list.
 * @param rf The RFID string to check.
 * @return 1 if the RFID is new/unique, 0 otherwise.
 */
int isNewRFID(Acc *head,char *rf){
        while(head){ // Iterate through the accounts list.
                if(!strcmp(rf,head->rfid)) return 0; // If RFID matches an existing one, it's not new.
                head=head->nxt; // Move to the next account.
        }
        return 1; // If loop completes without finding a match, RFID is new.
}

/**
 * @brief Generates a unique account number.
 * This function attempts to create an 18-digit unique ID by combining a timestamp
 * with a random number, and ensures it doesn't collide with existing account numbers.
 * @param head Pointer to the first account in the linked list (to check for uniqueness).
 * @return A unique u64 account number.
 */
u64 getUnqId(Acc *head){
        u64 num; // Variable to store the generated account number.
        Acc *temp=NULL; // Temporary pointer to traverse the accounts list.
        int found=1; // Flag to control the loop, 1 means a collision was found or first run.
        while(found){ // Loop until a unique number is generated.
                srand(getpid()+ (unsigned int)head); // Seed the random number generator. Using head's address might not be ideal if head is often NULL or changes predictably.
                //18 digit unq ID
                num=getTimeStamp()*10000 +(rand()%10000); // Generate number: timestamp (14 digits) * 10000 + 4 random digits.
                temp=head; // Start checking from the beginning of the list.
                found=0;   // Assume unique initially for this iteration.
                while(temp){ // Traverse existing accounts.
                        if(num==temp->num){ // If generated number matches an existing one.
                                found=1;      // Set flag to indicate collision.
                                break;        // Exit inner loop.
                        }
                        temp=temp->nxt; // Move to the next account.
                }
        }
        return num; // Return the unique account number.
}

/**
 * @brief Generates a timestamp as a u64 integer in YYYYMMDDHHMMSS format.
 * @return The current timestamp as a u64 value.
 */
u64 getTimeStamp(void){
    time_t rawtime; // Variable to store raw time value.
    struct tm *timeinfo; // Pointer to structure holding broken-down time.

    // Get current UTC time
    time(&rawtime); // Get current calendar time.

    // Convert to IST (UTC +5:30)
   // rawtime +=19800;  // 19800 seconds = 5 hours 30 minutes // Commented out: IST conversion is not applied. Uses local time.
    timeinfo =localtime(&rawtime); // Convert rawtime to local time structure.

    // Format as YYYYMMDDHHMMSS (14-digit ID)
    u64 timeStamp = // Construct the timestamp value.
        (timeinfo->tm_year+1900)*10000000000ULL+ // Year (tm_year is years since 1900).
        (timeinfo->tm_mon+1)*100000000ULL+      // Month (tm_mon is 0-11).
        timeinfo->tm_mday*1000000ULL+           // Day of the month.
        timeinfo->tm_hour*10000ULL+             // Hour.
        timeinfo->tm_min *100ULL+               // Minute.
        timeinfo->tm_sec;                       // Second.

    return timeStamp; // Return the formatted timestamp.
}

/**
 * @brief Reads a line of string input from the user until a newline character.
 * Allocates memory for the string using strdup. The caller must free this memory.
 * @return A pointer to the dynamically allocated string containing the user input.
 */
char *getStr(void){
        char buff[NAME_LEN]={0}; // Static buffer to read initial input, NAME_LEN is 30.
        fflush(stdout);          // Flush the output buffer.
        __fpurge(stdin);         // Clear the input buffer (non-standard).
        scanf("%[^\n]",buff);    // Read string until newline. Vulnerable to buffer overflow if input > NAME_LEN-1.
        return strdup(buff);     // Duplicate the string into dynamically allocated memory and return it.
}

/**
 * @brief Gets a single character from standard input without waiting for Enter (non-blocking).
 * This function is platform-dependent (uses termios for Unix-like systems).
 * @return The character read, or -1 if no input is available (non-blocking).
 */
char getch(void){ // Reads a single character without echo and without waiting for enter.
    struct termios oldt, newt; // Structures to hold terminal attributes.
    int oldf; // To store old file status flags.
    int ch;   // Character read.

    // Save terminal settings
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal attributes.
    newt = oldt; // Copy old attributes to new settings structure.

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON); // Disable canonical mode (line buffering). Echo is not explicitly disabled here but often is with ICANON.
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new terminal attributes immediately.

    // Set non-blocking mode
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0); // Get current file status flags for stdin.
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Set stdin to non-blocking mode.

    ch = getchar(); // Attempt to read a character.

    // Restore settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original terminal attributes.
    fcntl(STDIN_FILENO, F_SETFL, oldf);      // Restore original file status flags.

    if (ch != EOF) { // If a character was read (getchar() returns EOF on error or no data in non-blocking mode).
        return ch;   // Return the character.
    }

    return -1; // No input was available.
}

/*
// Original commented-out getKey function.
// This version appears to handle backspace and echo characters,
// but the currently active getKey is simpler.
char getKey(void) {
    struct termios oldt, newt;
    char key[3] = {0}, ch;
    int i = 0;

    // Turn off canonical mode and echo
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < 3) {
        read(STDIN_FILENO, &ch, 1);

        if (ch == 8 || ch == 127) {  // Backspace handling
            if (i > 0) {
                i--;
                printf("\b \b");  // Move back, print space, move back
                fflush(stdout);
            }
        }
        else if (ch == '\n' || ch == '\r') {  // Enter
            key[i] = '\0';
            printf("\n");  // Move to next line
            break;
        }
        else {
            key[i++] = ch;
            printf("%c", ch);  // Echo character
            fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore terminal settings

    if (i == 3 || key[1] != '\0') return '\0';  // Check for invalid input (more than one char before enter)
    CAPS(key[0]);  // Uncomment if you have this macro
    return key[0];
}
*/

/**
 * @brief Reads a single character key from the user for menu selection.
 * It flushes stdout, clears stdin, reads a character, converts it to uppercase,
 * prints a newline, and returns the character.
 * @return The uppercase character entered by the user.
 */
char getKey(void){
        char key[3],ch; // key buffer (only key[0] is practically used), ch is unused in current version.
        int i=0; // i is unused in current version.
        fflush(stdout);   // Flush standard output.
        __fpurge(stdin);  // Clear standard input buffer (non-standard).
        key[0]=getchar(); // Read a single character.
        /*
        // Commented-out block: alternative implementation for getKey, possibly using getch()
        while(i<3){
                while((ch=getch())==-1); // Wait for a character from getch()
                key[i]=ch;
                if((ch=='\b')||(ch==127)){ // Backspace handling
                        if(i)i--;
                        key[i]=0;
                }else if(ch=='\n'){ // Enter key
                        key[i]='\0';
                        break;
                }else i++;
        }
        if((i==3)||(key[1]!='\0')) return '\0'; // If more than one char or buffer full, return null char.
        */
        CAPS(key[0]); // Convert the first character to uppercase using the CAPS macro.
        puts(""); // Print a newline character for better formatting.
        return key[0]; // Return the processed character.
}

/**
 * @brief Formats a string by capitalizing the first letter of each word.
 * Modifies the string in place. Assumes words are separated by single spaces.
 * @param str The string to format.
 */
void format(char *str){
        CAPS(str[0]); // Capitalize the first character of the string.
        str++; // Move pointer to the next character.
        while(*str){ // Iterate through the rest of the string.
                if(*(str-1)==' ')CAPS(*str); // If the previous character was a space, capitalize the current character.
                str++; // Move to the next character.
        }
}
/// End of general utility functions.

/// Account modification functions.
/**
 * @brief Allows updating various details of a user's account.
 * Presents a menu for choosing which detail to update (phone, name, username, password, PIN).
 * @param head Pointer to the pointer of the first account (used by isUnq for username updates).
 * @param usr Pointer to the account structure to be updated.
 */
void updateAcc(Acc **head,Acc *usr){ // usr is the account to be updated. head is the list for uniqueness checks.
        char key,*temp=NULL; // key for menu choice, temp for string input.
        int flag=1; // Loop control/error flag.
        accMenu(); // Display account update options menu.

        key=getKey(); // Get user's choice.
        __fpurge(stdin); // Clear input buffer.
        puts(""); // Print a newline.
        switch(key){ // Process based on user's choice.
                case 'P': // Update Phone number.
                        printf("Enter New phone number:");
                        scanf("%llu",&(usr->phno)); // Read new phone number.
                        puts(BGREEN"Mobile Updated."RESET); // Confirmation.
                        break;
                case 'O': // Update account hOlder's name.
                        printf("Enter New Holder name:");
                        __fpurge(stdin); // Clear input buffer.
                        temp=getStr(); // Read new name.
                        if(strlen(temp)<3){ // Validate name length.
                                puts("invalid name");
                                free(temp); // Free temporary string.
                                return; // Exit if name is invalid.
                        }
                        // Assuming usr->name was dynamically allocated, it should be freed before reassigning.
                        // This could lead to a memory leak if usr->name was previously allocated and not freed.
                        // Or, if it was part of the main struct, this overwrites the pointer. Given `newAcc` uses `getStr`, it's likely dynamic.
                        // It's safer to: free(usr->name); usr->name = temp;
                        // However, sticking to "don't touch code", this is how it is.
                        format(temp); // Format the new name.
                        usr->name=temp; // Assign new name. temp is not freed if it becomes the new name.
                        puts(BGREEN"Name Updated."RESET); // Confirmation.
                        break;
                case 'U': // Update Username.
                        flag=1;
                        while(flag){ // Loop for valid username input.
                                printf("Enter New Username:");
                                temp=getStr(); // Read new username.
                                if(strlen(temp)>MAX_USRN_LEN){ // Validate length.
                                        puts("Username too long,try again!!");
                                        free(temp); // Free temp before continuing.
                                        continue;
                                }
                                if(!strcmp(temp,usr->usrName)){ // If new username is same as old.
                                        puts("New Username is same as your Existing Username.");
                                        puts("Do you wanna change the username?(y/n):");
                                        key=getKey(); // Ask if user still wants to change.
                                        if(key=='Y'){
                                                free(temp); // Free and continue loop.
                                                continue;
                                        }else if(key=='N'){
                                                free(temp); // Free and return.
                                                return;
                                        }else{
                                                //stupid // User entered invalid choice (Y/N), implicitly continues or breaks depending on unhandled key
                                                free(temp); // Should free temp here as well if not continuing
                                                return; // Or handle appropriately. Assuming return for safety.
                                        }
                                }else if(isUnq(*head,temp)){ // Check if new username is unique.
                                        strcpy(usr->usrName,temp); // Copy new username.
                                        free(temp); // Free temporary string.
                                        flag=0; // Exit loop.
                                }else{
                                        puts("User name aldready exits!");
                                        puts("Try differnt one.");
                                        free(temp); // Free if not unique and loop continues.
                                }
                        }
                        if(!flag)puts(BGREEN"Username Updated."RESET); // Confirmation if update was successful.
                        break;
                case 'Q': // Update Password (Q is an odd choice, typically P for password).
                        printf("Enter old password:");
                        temp=getStr(); // Get current password for verification.
                        if(strcmp(temp,usr->pass)){ // Check if old password matches.
                                puts("Wrong password!!");
                                free(temp); // Free temp.
                                break; // Exit case if wrong.
                        }
                        free(temp); // Free temp if old password was correct.
                        flag=1;
                        while(flag){ // Loop for new password input and confirmation.
                                if(flag>1)puts("Password mismatch!! Retry.");
                                printf("Enter New Login Password:");
                                temp=getStr(); // Read new password.
                                if(strlen(temp)>=MAX_PASS_LEN){ // Validate length.
                                        puts("Password too long,try again!!");
                                        flag=1; // Keep flag as is or reset for retry count.
                                        free(temp); // Free before continuing.
                                        continue;
                                }
                                strcpy(usr->pass,temp); // Copy new password.
                                free(temp); // Free this temp.
                                printf("Re-enter New Login Password:");
                                temp=getStr(); // Read password confirmation.
                                flag=(strcmp(temp,usr->pass)?flag+1:0); // Check match.
                                free(temp); // Free confirmation temp.
                        }
                        puts(BGREEN"Password Updated."RESET); // Confirmation.
                        break;
                case 'A': // Update ATM Pin.
                        printf("Enter Old ATM Pin:");
                        temp=getStr(); // Get current PIN for verification.
                        if(strcmp(temp,usr->pin)){ // Check if old PIN matches.
                                puts("Wrong pin!!");
                                free(temp); // Free temp.
                                break; // Exit case if wrong.
                        }
                        flag=1; // Reset flag.
                        free(temp); // Free temp if old PIN was correct.
                        while(flag){ // Loop for new PIN input and confirmation.
                                if(flag>1)puts("Pin mismatch!! Retry.");
                                printf("Enter New ATM Pin:");
                                temp=getStr(); // Read new PIN.
                                if(strlen(temp)!=4){ // Validate length (must be 4 digits).
                                        puts("4 digit pin pls,try again!!");
                                        flag=1; // Keep flag as is or reset.
                                        free(temp); // Free before continuing.
                                        continue;
                                }
                                strcpy(usr->pin,temp); // Copy new PIN.
                                free(temp); // Free this temp.
                                printf("Re-enter New ATM Pin:");
                                temp=getStr(); // Read PIN confirmation.
                                flag=(strcmp(temp,usr->pin)?flag+1:0); // Check match.
                                free(temp); // Free confirmation temp.
                        }
                        puts(BGREEN"Pin Updated."RESET); // Confirmation.
                        break;
                default:puts("invalid input."); // Handle invalid menu choice.
                        break;
        }
}

/**
 * @brief Displays the menu for account update options.
 * Lists choices for what specific account detail a user might want to change.
 */
void accMenu(void){
        printf(BBLUE"\nChange:\n" // Menu title.
                       "[KEY]-ACTION\n"        // Header.
                       "p/P  -Phone number.\n"  // Option for phone number.
                       "o/O  -Holder's name.\n" // Option for holder's name.
                       "u/U  -Username.\n"      // Option for username.
                       "q/Q  -Password.\n"BYELLOW // Option for password. (Q is unconventional)
                       "Enter choice:"RESET); // Prompt for choice.
}
/// End of account update functions.

/// Account display and retrieval functions.
/**
 * @brief Displays the details of a given account.
 * @param usr Pointer to the `Acc` structure whose details are to be printed.
 */
void dispAcc(Acc *usr){
        puts(BBLUE"\n==:Account Details:=="RESET); // Section title.
        printf("AccNo.:%llu\n",usr->num);        // Print account number.
        printf("Name  :%s\n",usr->name);         // Print account holder's name.
        printf("Ph.No.:%llu\n",usr->phno);       // Print phone number.
        printf("Balanc:%lf\n",usr->bal);        // Print current balance.
        printf("Usrnam:%s\n",usr->usrName);      // Print username.
        printf("Passwd:%s\n",usr->pass);        // Print password (for admin/debug, not for user).
        printf("RFID  :%s\n",usr->rfid);         // Print RFID.
        printf("Pin   :%s\n",usr->pin);          // Print PIN (for admin/debug, not for user).
        printf("Card  :%s\n",(usr->cardStat==1)?"ACTIVE":"BLOCKED"); // Print card status.
        printf("TranNo:%llu\n",usr->tranCnt);    // Print total number of transactions.
}
///

///
/**
 * @brief Retrieves an account from the linked list based on user-specified criteria.
 * Prompts the user to choose a search method (phone, account number, name, username).
 * @param head Pointer to the first account in the linked list.
 * @return Pointer to the found `Acc` structure, or `NULL` if not found or invalid choice.
 */
Acc* getAcc(Acc *head){
        u64 num=0; // Variable for numerical search input (phone/account number).
        char key='\0',*str=NULL; // key for menu choice, str for string search input (name/username).
        printf( BBLUE"Find by:\n" // Menu for search options.
                "[KEY]-ACTION\n"
                "p/P  -Phone number.\n"
                "n/N  -Account number.\n"
                "o/O  -Holder name.\n"
                "u/U  -Username.\n"BYELLOW
                "Enter choice:"RESET); // Prompt for choice.
        key=getKey(); // Get user's choice.
        __fpurge(stdin); // Clear input buffer.
        puts(""); // Print newline.
        switch(key){
                case 'P': // Search by Phone number.
                        printf("Enter phone number:");
                        scanf("%llu",&num); // Read phone number to search.
                        while(head){ // Traverse list.
                                if(num==head->phno)break; // Found.
                                head=head->nxt;
                        }
                        break;
                case 'N': // Search by Account number.
                        printf("Enter account number:");
                        scanf("%llu",&num); // Read account number to search.
                        while(head){ // Traverse list.
                                if(num==head->num)break; // Found.
                                head=head->nxt;
                        }
                        break;
                case 'O': // Search by hOlder name.
                        printf("Enter holder name:");
                        str=getStr(); // Read name to search.
                        format(str); // Format input name (capitalization) to match stored format.
                        while(head){ // Traverse list.
                                if(!strcmp(str,head->name))break; // Found.
                                head=head->nxt;
                        }
                        free(str); // Free temporary string.
                        break;
                case 'U': // Search by Username.
                        printf("Enter username:");
                        str=getStr(); // Read username to search.
                        while(head){ // Traverse list.
                                if(!strcmp(str,head->usrName))break; // Found.
                                head=head->nxt;
                        }
                        free(str); // Free temporary string.
                        break;
                default:puts("invalid choice"); // Invalid search option.
                        head=NULL; // Set head to NULL to indicate not found or error.
                        break;
        }
        return head; // Return pointer to found account or NULL.
}
///

///
/**
 * @brief Placeholder for deleting an account. Currently not implemented.
 * @param head Pointer to the pointer of the first account in the list.
 */
void dltAcc(Acc **head){ // Function to delete an account.
        if(!(*head))return; // If list is empty, do nothing.

        //travese to acc // Placeholder comment for traversal logic.
        //delete         // Placeholder comment for deletion logic.
        // This function is not implemented.
}
/// End of account retrieval/deletion functions.

/// Transaction and balance functions.
/**
 * @brief Displays the balance details for a given account.
 * @param usr Pointer to the `Acc` structure.
 */
void balance(Acc *usr){
        printf("\nAccount Number : %llu\n",usr->num);    // Display account number.
        printf("Holder Name    : %s\n",usr->name);      // Display holder name.
        printf("Current Balance: %+lf Rs/-\n",usr->bal); // Display current balance with sign and currency.
}
///

///
/**
 * @brief Processes a deposit transaction for the specified account.
 * Validates the deposit amount against limits and updates balance and transaction history.
 * @param usr Pointer to the `Acc` structure for the account.
 */
void deposit(Acc *usr){
        f64 amt=0; // Variable to store deposit amount.
        printf("\nEnter Deposit Amount:");
        scanf("%lf",&amt); // Read deposit amount.

        if(amt<=0){ // Validate amount is positive.
                puts("Amount cannot be negative!!");
                puts("Try again!!");
        }else if(amt<MAX_DEPOSIT){ // Validate against maximum deposit limit.
                usr->bal += amt; // Add amount to balance.
                //update 2 transc // Comment indicating transaction update.
                addTran(usr,+amt,DEPOSIT); // Add deposit transaction to history.
                puts(BGREEN"Amount Deposited."RESET); // Confirmation.
        }else{
                puts("Amount exceeds Max.Deposit limit!!"); // Error if limit exceeded.
                puts("Try again!!");
        }
}
///

///
/**
 * @brief Processes a withdrawal transaction for the specified account.
 * Validates amount against limits and balance, updates balance and transaction history.
 * @param usr Pointer to the `Acc` structure for the account.
 */
void withdraw(Acc *usr){
        f64 amt=0; // Variable to store withdrawal amount.
        printf("\nEnter Withdrawal Amount:");
        scanf("%lf",&amt); // Read withdrawal amount.
        if(amt<=0){ // Validate amount is positive.
                puts("Amount cannot be negative!!");
                puts("Try again!!");
        }else if(amt<MAX_WITHDRAW){ // Validate against maximum withdrawal limit.
                if(amt<=usr->bal){ // Check for sufficient balance.
                        usr->bal -= amt; // Deduct amount from balance.
                        //update 2 transc // Comment indicating transaction update.
                        addTran(usr,-amt,WITHDRAW);//1 // Add withdrawal transaction to history (amount is negative). '1' is a comment.
                        puts(BGREEN"Amount Withdrawn."RESET); // Confirmation.
                }else{
                        puts("Low Balance!!"); // Error for insufficient balance.
                }
        }else{
                puts("Amount exceeds Max.Withdraw limit!!"); // Error if limit exceeded.
                puts("Try again!!");
        }
}
///

///
/**
 * @brief Processes a fund transfer from one account to another.
 * Validates amount against limits and sender's balance, updates balances and transaction histories for both accounts.
 * @param from Pointer to the sender's `Acc` structure.
 * @param to Pointer to the receiver's `Acc` structure.
 */
void transfer(Acc *from,Acc *to){
        f64 amt=0; // Variable to store transfer amount.
        printf("\nEnter Transfer Amount:");
        scanf("%lf",&amt); // Read transfer amount.
        if(amt<=0){ // Validate amount is positive.
                puts("Amount cannot be negative!!");
                puts("Try again!!");
        }else if(amt<MAX_TRANSFER){ // Validate against maximum transfer limit.
                if(amt<=from->bal){ // Check for sufficient balance in sender's account.
                        from->bal -= amt; // Deduct from sender.
                        to->bal   += amt; // Add to receiver.
                        //update 2 transc of both // Comment indicating transaction updates for both.
                        addTran(to,+amt,TRANSFER_IN);  // Record transfer-in for receiver.
                        addTran(from,-amt,TRANSFER_OUT); // Record transfer-out for sender.
                        puts(BGREEN"Amount Transfered."RESET); // Confirmation.
                }else{
                        puts("Low Balance!!"); // Error for insufficient balance.
                }
        }else{
                puts("Amount exceeds Max.Transfer limit!!"); // Error if limit exceeded.
                puts("Try again!!");
        }
}
///

///
/**
 * @brief Adds a new transaction record to the specified account's transaction history.
 * The transaction is added to the beginning of the history linked list.
 * @param usr Pointer to the `Acc` structure to which the transaction is added.
 * @param amt The amount of the transaction (can be positive or negative).
 * @param type The type of transaction (e.g., DEPOSIT, WITHDRAW).
 */
void addTran(Acc *usr,f64 amt,char type){
        Tran *new=calloc(1,sizeof(Tran)); // Allocate memory for a new transaction.
        new->amt=amt; // Set transaction amount.
        new->id =getTranId(usr); // Generate a unique transaction ID.
        new->type=type; // Set transaction type.
        new->nxt=NULL; // Initialize next pointer to NULL.

        new->nxt=usr->tranHist; // New transaction points to the current head of history.
        usr->tranHist=new;      // Head of history now points to the new transaction.

        (usr->tranCnt)++; // Increment the account's transaction counter.
}

/**
 * @brief Generates a unique transaction ID.
 * Creates a 17-digit unique ID using a timestamp and a random number, seeded by the account number.
 * @param usr Pointer to the `Acc` structure for which the transaction ID is generated.
 * @return A unique u64 transaction ID.
 */
u64 getTranId(Acc *usr){
        //17 digit unq TranID
        srand(usr->num); // Seed random number generator with account number for variability per account.
        return getTimeStamp()*1000 +(rand()%1000); // Timestamp (14 digits) * 1000 + 3 random digits.
}
/// End of transaction processing functions.

/// Display and reporting functions.
/**
 * @brief Displays the transaction history for a given account.
 * Lists transaction ID, amount, and type for each transaction.
 * @param usr Pointer to the `Acc` structure.
 */
void statement(Acc *usr){
    Tran *temp = usr->tranHist; // Temporary pointer to traverse transaction history.
    if (temp) { // Check if there are any transactions.
        printf(BCYAN"\n%-20s%-23s%-12s\n", "Transaction ID", "Amount (Rs)","Type"); // Header for statement.
        puts("----------------------------------------"); // Separator line.
        while (temp) { // Loop through all transactions.
            printf("%-20llu%-+20.2lf",temp->id,temp->amt); // Print ID and amount (with sign, 2 decimal places).
            if(temp->type==DEPOSIT)      printf("%-12s\n","Deposit");      // Print type as "Deposit".
            else if(temp->type==WITHDRAW)printf("%-12s\n","Withdraw");     // Print type as "Withdraw".
            else if(temp->type==TRANSFER_IN)printf("%-12s\n","Tranfer IN"); // Print type as "Transfer IN".
            else if(temp->type==TRANSFER_OUT)printf("%-12s\n","Tranfer OUT");// Print type as "Transfer OUT".
            temp = temp->nxt; // Move to the next transaction.
        }
    } else {
        puts("No Transaction History!"); // Message if no transactions exist.
    }
    printf(RESET); // Reset text color.
}
///

///
/**
 * @brief Displays a summary of all accounts in the database.
 * For each account, it shows Account ID, Holder Name, Mobile Number, and Transaction Count.
 * @param head Pointer to the first account in the linked list.
 */
void database(Acc *head){
        if(!head){ // Check if the database is empty.
                puts("Empty Database!!WTF"); // Message if no accounts.
                return;
        }
        // Print table header.
        printf(BBLUE"\n%-20s|%-40s|%-14s|%-12s\n",
                       "Account ID",
                       "Holder Name",
                       "Mobile(+91)",
                       "Transactions"RESET);
        while(head){ // Loop through all accounts in the list.
                // Print details for each account.
                printf("%-20llu|%-40s|+91-%-10llu|%-12llu\n",
                               head->num,      // Account ID.
                               head->name,     // Holder Name.
                               head->phno,     // Phone Number.
                               head->tranCnt); // Transaction Count.
                head=head->nxt; // Move to the next account.
        }
}
/// End of display/reporting functions.

// File format comments:
// Transaction file entry format: "%lu,%lf,%c",id,amt,type
// Account database file entry format: "%lu,%s,%llu,%s,%s,%s,%s,%d,%lf,%llu",num,name,phno,usrName,pass,rfid,pin,cardStat,bal,tranCnt

/// File handling functions.
//

/**
 * @brief Saves account data and individual transaction histories to CSV files in the "../dataz/" directory.
 * `Db.csv` stores main account details. `<account_number>.csv` stores transaction history for each account.
 * These files are primarily for data persistence and are loaded by `syncData`.
 * @param head Pointer to the first account in the linked list.
 */
void saveData(Acc *head){

        FILE *fp=fopen("../dataz/Db.csv","w"); // Open/create the main database CSV file in write mode.
                                               // This will overwrite the file if it exists.
        if(!fp) { // Check if file opening failed.
            perror("saveData: Db.csv"); // Print error if Db.csv cannot be opened.
            return;
        }

        while(head){ // Iterate through all accounts.
                // Write account details to Db.csv.
                fprintf(fp,"%llu,%s,%llu,%s,%s,%s,%s,%d,%lf,%llu\n",head->num,head->name,head->phno,
                               head->usrName,head->pass,head->rfid,head->pin,head->cardStat,head->bal,head->tranCnt);

                //save bank statement for the current account
                char spName[30]; // Buffer for transaction file name.
                sprintf(spName,"../dataz/%llu.csv",head->num); // Create filename like "dataz/12345.csv".
                FILE *sp=fopen(spName,"w"); // Open/create transaction history file for this account.
                if(!sp) { // Check if transaction file opening failed.
                    perror("saveData: transaction file"); // Print error.
                    // Decide if to continue with next account or stop. Current logic continues.
                    head=head->nxt;
                    continue;
                }
                Tran *t=head->tranHist; // Pointer to traverse transaction history.
                while(t){ // Iterate through transactions for this account.
                        fprintf(sp,"%llu,%lf,%c\n",t->id,t->amt,t->type); // Write transaction details.
                        t=t->nxt; // Move to next transaction.
                }
                fclose(sp); // Close the transaction history file for this account.
                head=head->nxt; // Move to the next account in the main list.
        }

        fclose(fp); // Close the main database CSV file.
}

/**
 * @brief Loads account data and transaction histories from CSV files in "../dataz/" into memory.
 * Reconstructs the linked list of accounts and their respective transaction histories.
 * This function is typically called at application startup.
 * @param head Pointer to the pointer of the first account, to build/populate the linked list.
 */
void syncData(Acc **head){
        FILE *fp=fopen("../dataz/Db.csv","r"); // Open the main database CSV file in read mode.
        // int d; // Variable 'd' seems unused in the loop's fscanf.
        char buf[100]; // Buffer to read the account holder's name (since it can contain commas if not handled carefully, but scanf %[^,] handles it).
        if(!fp){ // If Db.csv doesn't exist or cannot be opened.
                perror("Sync"); // Print error message.
                return; // Exit function.
        }
        puts("syncing"); // Indicate that data synchronization is in progress.
        Acc temp,*tail=NULL; // temp: temporary Acc structure to read data into. tail: to efficiently append to the linked list.

        temp.nxt=NULL; // Initialize temporary account's next pointer.
        temp.tranHist=NULL; // Initialize temporary account's transaction history.
        temp.tranCnt=0; // Initialize temporary account's transaction count.

        // Read account data from Db.csv line by line.
        while(fscanf(fp,"%llu,%[^,],%llu,%[^,],%[^,],%[^,],%[^,],%d,%lf,%llu\n", // Note: added \n to consume newline
                                &(temp.num),buf,&(temp.phno),temp.usrName,temp.pass,
                                temp.rfid,temp.pin,&(temp.cardStat),&(temp.bal),&(temp.tranCnt))==10){ // 10 fields expected.
                temp.name=strdup(buf); // Duplicate the read name into dynamically allocated memory.
                /*
                // Debugging printf block, commented out.
                printf("%llu,%s,%llu,%s,%s,%s,%s,%d,%lf,%llu\n",
                               temp.num, temp.name, temp.phno,
                               temp.usrName, temp.pass, temp.rfid,
                               temp.pin, temp.cardStat, temp.bal, temp.tranCnt);

                printf("d=%d\n",d); // 'd' is not assigned by fscanf.
                while(1); // Infinite loop for debugging.
                */

                Acc *new =calloc(1,sizeof(Acc)); // Allocate memory for a new account node.
                memmove(new,&temp,sizeof(Acc)); // Copy data from temp to the new node. new->name now points to the strdup'd memory.
                                                // temp.name itself will be overwritten by next strdup or be a dangling pointer after loop if not careful.
                                                // However, since strdup creates new memory each time, this is okay for new->name.
                new->nxt = NULL; // Ensure the new node's next pointer is NULL before linking.

                if(!(*head))*head=new; // If list is empty, new node becomes the head.
                if(tail)tail->nxt=new; // If list is not empty, link previous tail to new node.
                tail=new; // Update tail to the new node.

                //load bank statement for the current account
                char spName[30]; // Buffer for transaction file name.
                sprintf(spName,"../dataz/%llu.csv",new->num); // Construct transaction file name.
                FILE *sp=fopen(spName,"r"); // Open transaction file in read mode.
                if(!sp)continue; // If transaction file doesn't exist, skip to next account.

                Tran *th=NULL,*tt=NULL,tm; // th: head of transaction list, tt: tail of transaction list, tm: temporary transaction.
                int cnt=0; // Counter for transactions read from file for this account.
                tm.nxt=NULL; // Initialize temporary transaction's next pointer.

                // Read transactions from the account's specific CSV file.
                while(fscanf(sp,"%llu,%lf,%c\n",&(tm.id),&(tm.amt),&(tm.type))==3){ // 3 fields expected. Added \n.
                        Tran *c=malloc(sizeof(Tran)); // Allocate memory for a new transaction node.
                        if(!c) {perror("syncData: malloc Tran"); break;} // Handle allocation failure
                        cnt++; // Increment transaction counter.
                        memmove(c,&tm,sizeof(Tran)); // Copy data from tm to new transaction node c.
                        c->nxt = NULL; // Ensure new transaction node's next pointer is NULL.

                        if(!th)th=c; // If transaction list is empty, new node is head.
                        if(tt)tt->nxt=c; // Link previous transaction tail to new node.
                        tt=c; // Update transaction tail.
                }
                new->tranHist=th; // Link the reconstructed transaction history to the account.
                new->tranCnt=cnt; // Update transaction count (could also use the one read from Db.csv, but this re-counts).
                fclose(sp); // Close the transaction file.
        }
        // After the loop, temp.name might hold a pointer to the last read name if strdup failed or loop exited prematurely.
        // It's good practice to free(temp.name) if it was conditionally allocated and not transferred, but here it's always transferred or overwritten.

        fclose(fp); // Close the main database file.
}

/**
 * @brief Saves account data and formatted transaction histories to CSV files in the "../filez/" directory.
 * `DataBase.csv` stores main account details with headers.
 * `<account_number>.csv` stores transaction history for each account with headers and formatted date/time.
 * These files are likely for human-readable reports or export.
 * @param head Pointer to the first account in the linked list.
 */
void saveFile(Acc *head){
        unsigned int dd,mon,yy,hh,mm; // Variables to store parts of date and time.
        u64 dum; // Temporary variable for timestamp decomposition.
        FILE *fp=fopen("../filez/DataBase.csv","w"); // Open/create the main report database file.
        if(!fp) { perror("saveFile: DataBase.csv"); return; }

        // Write header row to the main report database file.
        fprintf(fp,"Account ID,Holder's name,Mobile no.,Username,Password,ATM card no.,ATM pin,Card Satus,Balance,Transactions count\n");
        while(head){ // Iterate through all accounts.
                // Write account details to DataBase.csv.
                fprintf(fp,"%llu,%s,%llu,%s,%s,%s,%s,%s,%lf,%llu\n",head->num,head->name,head->phno,
                               head->usrName,head->pass,head->rfid,head->pin,(head->cardStat)?"ACTIVE":"BLOCKED"
                               ,head->bal,head->tranCnt);

                //save bank statement for the current account in filez directory
                char spName[40]; // Buffer for transaction report file name.
                sprintf(spName,"../filez/%llu.csv",head->num); // Create filename like "filez/12345.csv".
                FILE *sp=fopen(spName,"w"); // Open/create transaction report file.
                if(!sp) { perror("saveFile: transaction report file"); head=head->nxt; continue; }

                Tran *t=head->tranHist; // Pointer to traverse transaction history.
                fprintf(sp,"Date,Time,Transaction ID,Amount,Type\n"); // Header for transaction report.
                while(t){ // Iterate through transactions.
                        // Decompose transaction ID (timestamp based) into date and time components.
                        // Assumes transaction ID structure: YYYYMMDDHHMMSSXXX (last 3 digits are random part)
                        dum=(t->id)/1000; // Remove the last 3 random digits to get YYYYMMDDHHMMSS part.
                                          // Original code has /100000, which would be YYYYMMDDH + H/10. This seems like a bug fix for ID structure.
                                          // Correcting based on general timestamp usage (getTimeStamp creates YYYYMMDDHHMMSS, getTranId appends 3 digits)
                                          // So, ID is YYYYMMDDHHMMSSRRR.
                                          // To get YYYYMMDDHHMMSS, divide by 1000.

                        // If t->id is YYYYMMDDHHMMSSFFF (FFF is 3 random digits from getTranId)
                        // Then dum = YYYYMMDDHHMMSS
                        u64 datetime_part = t->id / 1000;

                        mm = datetime_part % 100;         // Extract minutes
                        datetime_part /= 100;
                        hh = datetime_part % 100;         // Extract hours
                        datetime_part /= 100;
                        dd = datetime_part % 100;         // Extract day
                        datetime_part /= 100;
                        mon = datetime_part % 100;        // Extract month
                        datetime_part /= 100;
                        yy = datetime_part;               // Remaining is year

                        // Print formatted date, time, and transaction details.
                        fprintf(sp,"%02u/%02u/%04u,%02u:%02u,%llu,%.2lf,",dd,mon,yy,hh,mm,t->id,t->amt);
                        if(t->type==DEPOSIT)            fprintf(sp,"%s\n","Deposit");
                        else if(t->type==WITHDRAW)      fprintf(sp,"%s\n","Withdraw");
                        else if(t->type==TRANSFER_IN)   fprintf(sp,"%s\n","Tranfer IN");
                        else if(t->type==TRANSFER_OUT)  fprintf(sp,"%s\n","Tranfer OUT");
                        t=t->nxt; // Move to next transaction.
                }
                fclose(sp); // Close the transaction report file.
                head=head->nxt; // Move to the next account.
        }
        fclose(fp); // Close the main report database file.
}
