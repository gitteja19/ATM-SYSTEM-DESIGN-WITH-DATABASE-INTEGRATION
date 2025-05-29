/**
 * @file bank_main.c
 * @brief Main application file for the Jet Bank system.
 * This file contains the main function that drives the banking application,
 * handling user login, and routing to admin or customer specific functionalities.
 * It initializes data, manages user sessions, and interacts with the bankLib functions.
 */

#include <stdio.h>      // For standard input/output functions like printf, puts.
#include <stdio_ext.h>  // For __fpurge (non-standard, used for clearing input buffer).
#include <stdlib.h>     // For general utilities like exit, memory allocation (though not directly used much here).
#include <string.h>     // For string comparison functions like strcmp.
#include <sys/stat.h>   // For mkdir function (to create directories).
#include <sys/types.h>  // For types used by sys/stat.h (like mode_t for mkdir).
#include "bankLib.h"    // Includes the custom banking library header.


/**
 * @brief The main function for the banking application.
 * Initializes the system, handles user login (admin/customer),
 * and then enters a loop for either admin or customer operations based on credentials.
 * Manages the main application flow and session termination.
 * @return int Returns 0 on successful execution (though this program has infinite loops until exited).
 * Returns 1 if admin logs in with ADMIN_EXIT password.
 */
int main(){
        Acc *db=NULL,*from=NULL,*to=NULL; // db: pointer to the head of account database (linked list).
                                          // from: pointer to the logged-in user's account or sender's account.
                                          // to: pointer to the receiver's account in transfer operations.
        char key='\0',*pass=NULL,*usr=NULL; // key: stores user input for menu choices or login type.
                                            // pass: stores the entered password string.
                                            // usr: stores the entered username string.
        int bye=0; // Flag to control breaking out of inner admin/user menu loops.

        //create data folder is not present
        mkdir("../dataz", 0777); // Create 'dataz' directory if it doesn't exist (for persistent data storage).
                                 // 0777 gives read/write/execute permissions to all.
        mkdir("../filez", 0777); // Create 'filez' directory if it doesn't exist (for human-readable report files).
        syncData(&db);           // Load existing account data from files into the 'db' linked list.
        puts(BRED"Hello All!!"RESET); // Display a welcome message in bold red.
        puts("");                // Print an empty line for spacing.

        while(1){ // Main application loop (login loop).
                //wait for login
                loginMenu(); // Display the login prompt.
                printf(BYELLOW"Enter Username:"RESET); // Prompt for username in bold yellow.
                usr=getStr(); // Read the username string from input.
                printf(BYELLOW"Enter Password:"RESET); // Prompt for password in bold yellow.
                pass=getStr(); // Read the password string from input.

                if(!strcmp(usr,ADMIN_USRN)){ // Check if the entered username is the admin username.
                        if(!strcmp(pass,ADMIN_PASS))key='A'; // If password matches admin password, set key to 'A' (Admin).
                        else if(!strcmp(pass,ADMIN_EXIT))exit(1); // If password is the admin exit command, terminate program.
                }else if(from=isValid(db,usr,pass)) key='C'; // If not admin, validate as customer. If valid, 'from' gets account, key='C' (Customer).
                else{ // If credentials are not admin and not a valid customer.
                        puts(BRED"Invalid credentials!"RESET); // Display error message.
                        free(usr); usr = NULL; // Free allocated username string.
                        free(pass); pass = NULL; // Free allocated password string.
                        continue; // Skip to the next iteration of the login loop.
                }
                free(usr); usr = NULL; // Free username string after successful validation or admin check.
                free(pass); pass = NULL; // Free password string.

                if(key=='A'){ // If logged in as Admin.
                        bye=0; // Reset bye flag for the admin session.
                        while(1){ // Admin operations loop.
                                adminMenu(); // Display the admin menu.
                                key=getKey(); // Get admin's menu choice.
                                if(!db){ // Check if the database is empty.
                                        // Allow 'Create New Account' (C) or 'Quit' (Q) even if DB is empty.
                                        if((key=='C')||(key=='Q')); // Do nothing, proceed to switch.
                                        else{ // For other operations on an empty DB.
                                                puts(BRED"Empty DataBase!!"RESET); // Display error.
                                                continue; // Restart admin operations loop.
                                        }
                                }

                                // For operations requiring an existing account, prompt for customer/sender info.
                                if(key=='H'||key=='W'||key=='D'||key=='T'||key=='B'||key=='F'||key=='U'||key=='X'){
                                        puts(BGREEN"=== Enter Customer/Sender Info ==="RESET); // Prompt.
                                        from=getAcc(db); // Search for and get the specified account.
                                        if(!from){ // If account not found.
                                                puts("Not found!! Try again.");
                                                continue; // Restart admin operations loop.
                                        }
                                }
                                switch(key){ // Process admin's choice.
                                        case 'C':newAcc(&db); // Create a new account.
                                                 break;
                                        case 'U':updateAcc(&db,from); // Update an existing account.
                                                 break;
                                        case 'H':statement(from); // View transaction history for an account.
                                                 break;
                                        case 'W':withdraw(from); // Perform withdrawal for an account.
                                                 break;
                                        case 'D':deposit(from); // Perform deposit for an account.
                                                 break;
                                        case 'B':balance(from); // Check balance for an account.
                                                 break;
                                        case 'X': // Activate/Deactivate card for an account.
                                                 if(from->cardStat !=1){ // If card is currently BLOCKED.
                                                         puts("Card status :BLOCKED");
                                                         puts("Activate card?(y/n)");
                                                         key=getKey(); // Get confirmation.
                                                         if(key=='Y'){
                                                         from->cardStat=1; // Activate card.
                                                         puts("card is activated.");
                                                         }
                                                 }else{ // If card is currently ACTIVE.
                                                         puts("Card status :ACTIVE");
                                                         puts("de-Activate card?(y/n)");
                                                         key=getKey(); // Get confirmation.
                                                         if(key=='Y'){
                                                         from->cardStat=0; // Deactivate (block) card.
                                                         puts("card is blocked.");
                                                         }
                                                 }
                                                 break;
                                        case 'T': // Transfer funds.
                                                 puts(BGREEN"==:Enter Receiver's info:=="RESET); // Prompt for receiver.
                                                 to  =getAcc(db); // Search for and get receiver's account.
                                                 if(!to){ // If receiver account not found.
                                                         puts("Not found!! Try again.");
                                                         break; // Break from switch, will continue admin loop.
                                                 }
                                                 transfer(from,to); // Perform the transfer.
                                                 break;
                                        case 'E':database(db); // Display all accounts in the database.
                                                 break;
                                        case 'F':dispAcc(from); // Display details of a specific (found) account.
                                                 break;
                                        case 'Q':saveData(db); // Save all data to persistent storage.
                                                 saveFile(db); // Save data to human-readable report files.
                                                 bye=1; // Set flag to exit admin operations loop.
                                                 break;
                                        default :puts("invalid option!."); // Invalid menu choice.
                                                 break;
                                }
                                if(bye){ // If bye flag is set (admin chose to quit).
                                        puts(BWHITE"Thank you for your work,Admin <3"RESET); // Farewell message.
                                        break; // Exit admin operations loop (returns to login loop).
                                }
                        }
                }else if(key=='C'){ // If logged in as Customer. 'from' already points to customer's account.
                        bye=0; // Reset bye flag for the customer session.
                        while(1){ // Customer operations loop.
                                userMenu(); // Display the customer menu.
                                key=getKey(); // Get customer's menu choice.
                                switch(key){ // Process customer's choice.
                                        case 'H':statement(from); // View own transaction history.
                                                 break;
                                        case 'W':withdraw(from); // Perform withdrawal from own account.
                                                 break;
                                        case 'D':deposit(from); // Perform deposit to own account.
                                                 break;
                                        case 'B':balance(from); // Check own account balance.
                                                 break;
                                        case 'T': // Transfer funds from own account.
                                                 puts("==:Enter Receiver's info:=="); // Prompt for receiver.
                                                 to  =getAcc(db); // Search for and get receiver's account.
                                                 if(!to){ // If receiver account not found.
                                                          puts("Not found!! Try again.");
                                                          break; // Break from switch, will continue customer loop.
                                                 }
                                                 transfer(from,to); // Perform the transfer.
                                                 break;
                                        case 'Q':saveData(db); // Save all data (customer action might also trigger global save).
                                                 saveFile(db); // Save report files.
                                                 bye=1; // Set flag to exit customer operations loop.
                                                 break;
                                        default :puts("invalid option!."); // Invalid menu choice.
                                                 break;
                                }
                                if(bye){ // If bye flag is set (customer chose to quit).
                                        puts("Happy to help, bye!! :)"); // Farewell message.
                                        break; // Exit customer operations loop (returns to login loop).
                                }
                        }
                }
        } // End of main application loop.
} // End of main function.
