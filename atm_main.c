#include "atmLib.h" //Includes the atmLib.h header file which contains declarations for ATM functions and structures

//The main function: entry point of the ATM simulation program.
//It initializes the system, handles communication, and processes ATM operations.
int main(){

        //local vars //Declaration of local variables used within the main function
        int fd,type; 
        //fd: file descriptor for serial communication, type: variable for operation type (currently unused)
        char buf[100]; 
        //buf: buffer to store received messages from UART
        Acc *db=NULL; 
        //db: pointer to the head of the linked list storing account data, initialized to NULL
        //Section for data synchronization
        syncData(&db);
        //Calls the function to load account data from storage into the 'db' linked list
#ifdef DBG //Conditional compilation block for debugging
        puts("synced"); //Prints "synced" to the console if DBG is defined, indicating data synchronization is complete
#endif //End of DBG conditional block
        //initiate uart //Section for initializing UART communication
        fd=initSerial(); //Calls the function to initialize serial (UART) communication and get the file descriptor
#ifdef DBG //Conditional compilation block for debugging
        puts("super loop"); //Prints "super loop" to the console if DBG is defined, indicating the start of the main processing loop
#endif //End of DBG conditional block

        //recv from uart and do necessary //Main processing loop: continuously receives data from UART and acts accordingly
        while(1){ //Infinite loop to keep the ATM operational
                rx_str(fd,buf,sizeof(buf)); //Receives a string from the serial port (fd) into 'buf', with a max size of 'sizeof(buf)'
         
                if(!isMsgOk(buf))continue; //Checks if the received message 'buf' is in the correct format; if not, skips to the next iteration
                //#<opt>:<data>$ //Expected message format: '#' followed by option, ':', data, and '$'
                switch(buf[1]){ //Switch statement based on the second character of the buffer (the option code)
                        //Case for RFID check operation
                        case 'C':checkRFID(db,fd,buf); //If option is 'C', calls the checkRFID function
                                 break; //Exits the switch statement
                        //Case for PIN verification operation
                        case 'V':verifyPin(db,fd,buf); //If option is 'V', calls the verifyPin function
                                 break; //Exits the switch statement
                        //Case for performing an ATM action
                        case 'A':puts("acting."); //If option is 'A', prints "acting." to the console
                                 act(db,fd,buf); //Calls the 'act' function to perform the specified ATM action
                                 break; //Exits the switch statement
                        //Case for checking the connection status
                        case 'X':tx_str(fd,"@X:LINEOK$"); //If option is 'X', sends a "LINEOK" message back
                                 break; //Exits the switch statement
                        case 'Q': //Case for quit/save operation
                                 saveData(db); //Saves the current account data to the primary data file (Db.csv)
                                 saveFile(db); //Saves the current account data to a more human-readable file (DataBase.csv)
                                 puts("data saved"); //Prints "data saved" to the console
                                 break; //Exits the switch statement
                } //End of switch statement
        } //End of while loop
} //End of main function
