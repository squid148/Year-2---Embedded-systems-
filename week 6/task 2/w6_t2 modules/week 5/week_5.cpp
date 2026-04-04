//=====[Libraries]=============================================================  //keep working on header files 
#include "Week_4.h"
#include "mbed.h"
#include "arm_book_lib.h"
#include "Week_5.h"
#include "display.h"
#include <cstdlib>
// #include "Week_4.h" //just double check if we need that

//=====[Defines]===============================================================
#define Debounce_Time_ms 400

//=====[Declaration of public data types]======================================

typedef enum { //declare structs/ datatypes ? what is enum for? electrical number
    KEYPAD_SCANNING, 
    KEYPAD_DEBOUNCE,
    KEYPAD_KEY_HELD
} KeypadState_t; //struct name

typedef struct systemEvent { //event log sctruct datatype that stores time of event and event that occurd
    time_t seconds;
    char typeOfEvent[EVENT_NAME_MAX_LENGTH]; //struct stores 
} systemEvent_t;
//=====[Declaration and init ialization of public global objects]===============

DigitalOut keypadRowPins[KEYPAD_NUMBER_OF_ROWS] = {PB_3, PB_5, PC_7, PA_15}; //declaring keypad row button array
DigitalIn keypadColPins[KEYPAD_NUMBER_OF_COLS]  = {PB_12, PB_13, PB_15, PC_6}; //declaring keypad column button array


//=====[Declaration and initialization of public global variables]=============

//Code check variables
int numberOfIncorrectCodes = 0; //records number of incorrect codes inputted
int numberOfHashKeyReleasedEvents = 0; //how many has has the hash key been used?
int keyBeingCompared    = 0; // variable to store current passcode array position vairable so it can be comapared to the inputted array
char codeSequence[NUMBER_OF_KEYS]   = { '2', '9', '4', '3' }; //passcode array to store correct passcode
char LCDReportentry[NUMBER_OF_KEYS] = {'4','5', '0', '0' };//added this code so the user can input 4 and 5 respectively and enter to get LCD report, W6_task 1 related,
// so in the case of 45 being entered need to set seperate route where command is called to display the current alarm states on the LCD, make new function called LCD report
char keyPressed[NUMBER_OF_KEYS] = { '0', '0', '0', '0' }; //stores user input
int accumulatedTimeAlarm = 0; //how many times has incorrect passcode been set off?

//alarm and sensor last state variables
bool alarmLastState        = OFF;
bool gasLastState          = OFF;
bool tempLastState         = OFF;
bool ICLastState           = OFF;//incorrect code last state
bool SBLastState           = OFF; //system block led

//event storage variables 
int eventsIndex = 0; 
systemEvent_t arrayOfStoredEvents[EVENT_MAX_STORAGE]; //array of stored events in array systemevent structures format 

//keypad  variables
int accumulatedDebounceKeypadTime = 0; //no clue, maybe 
int KeypadCodeIndex = 0; //identifies which key is being pressed
char KeypadLastKeyPressed = '\0'; //null symbol to end population of user input array
char KeypadIndexToCharArray[] = { //just one array with no rows or columns just one continuos array
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D',
};

//incorrect code variables
bool incorrectCode = false; 

//time variables
bool time_set = false;//starts as off and turns on when user populates time struct at beginning of code

//datatype declare
KeypadState_t KeypadState;//keypadstate_t struct data type created called KeypadState



// ===[function]====================================================

void InputsInit(){ 
LM35ReadingsArrayInit(); //most recent update, do next: add in necessary function to fix inputs initialization, keypad and lm35readingarray.
KeypadInit();
}


/*
bool code_check(){ //code that compares user inputted code with passcode to turn off alarm
for(int i=0;i<4;i++){
    if ()
}
return true;
}
*/


//just here to act as an example
void KeypadInit() //
{
    KeypadState = KEYPAD_SCANNING; //part of enum struct datatype
    for( int pinIndex=0; pinIndex<KEYPAD_NUMBER_OF_COLS; pinIndex++ ) { //initializes makes all keypadbuttons to pull up mode?
        (keypadColPins[pinIndex]).mode(PullUp); //initializes all keypad columns as pullups
    }
}

char KeypadUpdate()
{
    char keyDetected = '\0'; //initiate null for both
    char keyReleased = '\0';

    switch( KeypadState ) {//takes the state of the matrix keypad to decide code to execute


    case KEYPAD_SCANNING: //by deafult starts in scanning mode of FSM
        keyDetected = KeypadScan(); 
        if( keyDetected != '\0' ) { //if a button is pressed and a non null value run following code
            KeypadLastKeyPressed = keyDetected;//sets last key pressed to current key being pressed
            accumulatedDebounceKeypadTime = 0;//when button is pressed
            KeypadState = KEYPAD_DEBOUNCE; //moves onto next state which is debounce
        }
        break;

    case KEYPAD_DEBOUNCE: //debounce state
        if( accumulatedDebounceKeypadTime >= Debounce_Time_ms)//checks to see if debounce time has elapsed
            {
            keyDetected = KeypadScan();
            if( keyDetected == KeypadLastKeyPressed ) {//checks to see if same button is being pressed
                KeypadState = KEYPAD_KEY_HELD;//if true transition onto hold state
            } else {
                KeypadState = KEYPAD_SCANNING;//reset if key isn't being pressed anymore 
            }
        }
        accumulatedDebounceKeypadTime = accumulatedDebounceKeypadTime + TIME_INCREMENT_MS;//always adds 10ms for each loop until debounce time has passed
        //always adds 10ms for each loop until debounce time has passed need to add time increment define for this to work 
        break;

    case KEYPAD_KEY_HELD:
    keyDetected = KeypadScan();
    if( keyDetected != KeypadLastKeyPressed ){
        if( keyDetected == '\0' ){
            keyReleased = KeypadLastKeyPressed;
            
            // debug print
            char str[50];
            sprintf(str, "Key released: %c\r\n", keyReleased); //prints out released key so you know what key you've pressed
            uartUsb.write(str, strlen(str));
        }
        KeypadState = KEYPAD_SCANNING;
    }
    break;

    default:
        KeypadInit(); //deafults in looking for scan when nothing has been pressed
        break;
    }
    return keyReleased;
}

char KeypadScan(){ //has to be char since keypad can return more than just numbers
    for (int row=0; row<KEYPAD_NUMBER_OF_ROWS; row++)    { //loops through rows
        for(int i=0; i<KEYPAD_NUMBER_OF_ROWS; i++){
            keypadRowPins[i]=ON; //turns on all keypad rows
        }
        
        keypadRowPins[row]=OFF; //A row is then turned off, the reason for this is if a button is pressed on a row that is off
        //when it's in pullup mode it will cause the button to output LOW meaning the row and column can be read by the board
        //works because when we assign a row as low it acts a sink so whenever a button is pushed it causes that column to also turn off due to contact
        for (int col=0; col<KEYPAD_NUMBER_OF_COLS; col++){
            if(keypadColPins[col] == OFF){//checks to see if column has been affected by sinked row causing it to be off
                return KeypadIndexToCharArray[row*KEYPAD_NUMBER_OF_ROWS + col];//return the keypad value at position of row and column
            }
        }
    

    }
    return '\0';//return null if no button is presed
}

void alarmdeactivate(){
    char str[100]; int StringLength;//might not work but unsure need to test
    //add if numberofincorrectcodes<x for system block
    char KeyReleased = KeypadUpdate(); //the key released means the key pressed
    if (KeyReleased != '\0' && KeyReleased != '#' && KeyReleased != '*' && KeyReleased != 'D'){ //key isn't null and # (enter), can't be or otherwise it would always be true in idle

        if (KeypadCodeIndex >= NUMBER_OF_KEYS) { //if greater than size of array
            KeypadCodeIndex = 0; //go back to start of passcode array
            
        }
        keyPressed[KeypadCodeIndex] = KeyReleased;
         KeypadCodeIndex++;//increase index by 1 and populate next member
        codeprint(); //moved code print to here so the digit codes are printed
    }
    if (KeyReleased ==  'D'){ //enter route
        
         if (alarmState){
                if (areEqual()) { //if code is correct reset system
                    alarmState = OFF;
                    incorrectCodeLed=OFF;
                    numberOfIncorrectCodes = 0;
                    KeypadCodeIndex = 0;
                    sirenPin=OFF;
                    sprintf(str,"\rcorrect code, alarm reset and system paused\n"); //code is 2943
                    StringLength=strlen(str);
                    uartUsb.write(str,StringLength);
                    codewipe();//wipe entered code on sucessfull run, comment out for debugging
                    delay(30000); //freeze for 30 seconds
                } 
                }
                else {
                    sprintf(str, "\rIncorrect code entered\n");
                    StringLength = strlen(str);
                    uartUsb.write(str, StringLength);
                    arrayOfStoredEvents[eventsIndex].seconds = time(NULL);
                    strncpy(arrayOfStoredEvents[eventsIndex].typeOfEvent, "IC ON", EVENT_NAME_MAX_LENGTH); //I couldn't get the eventlogupdate to pick up on incorrect code triggers so I just wrote stright into the event array using the same code
                    if (eventsIndex < EVENT_MAX_STORAGE - 1){
                    eventsIndex++;
                    } else {
                        eventsIndex = 0;
                        }
                    for (int i = 0; i < 6; i++){ //led blinks to show incorrect code
                        incorrectCodeLed = !incorrectCodeLed;
                        delay(500);
                    }
                    incorrectCodeLed = OFF;  //ensures led is off
                    numberOfIncorrectCodes++;
                        
                    }
                }
            
    if (KeyReleased == '*') { //clear route
        codewipe();//wipes entered code
    }
                    if (KeyReleased == '#') {
        //code to print out event log in correct password entry
                    for (int i=0;i<eventsIndex;i++){ //prints out 
                        sprintf(str, "Event: %s\r\n", arrayOfStoredEvents[i].typeOfEvent);
                        uartUsb.write(str, strlen(str));
                        sprintf(str, "Date and time: %s\r\n", ctime(&arrayOfStoredEvents[i].seconds));
                        uartUsb.write(str, strlen(str));
                        uartUsb.write("\r\n", 2);
                        }
                        if (eventsIndex == 0){
                        uartUsb.write("No recorded events\r\n", 20);
                        }
                    }
            //don't need system block yet
}
    


bool areEqual(){ //could edit arequal to return an integer that enters a switch case to run according code. ex, compares number of keys if correct return 1, else if check for 45/report,
//if correct return 2 and if else teturn 0
    int i;

    for (i = 0; i < NUMBER_OF_KEYS; i++) {
        if (codeSequence[i] != keyPressed[i]) {
            return false;
        }
    }

    return true;
}

bool LCDReportRequest(){ //this function checks to see if 45 has been entered and if true returns true else if something else return false
    int i;

    for (i = 0; i < NUMBER_OF_KEYS; i++) {
        if (LCDReportentry[i] != keyPressed[i]) {
            return false;
        }
    }

    return true;
}


void codeprint(){
    char str[50]; int StringLength;//might not work but unsure need to test
    sprintf(str,"\rentered code: %c%c%c%c\n----------------------------\n", keyPressed[0],keyPressed[1],keyPressed[2],keyPressed[3]); //code is 1805
            StringLength=strlen(str);
            uartUsb.write(str,StringLength);
}

void codewipe(){
for (int j=0;j<4;j++){
        keyPressed[j]='0'; //clears the entire entered to code, don't put just 0 otherwise it registers as null which breaks the system
        }
    KeypadCodeIndex=0;//sets user back to beginning of code array 
    }


 
void timesetter() { //have to change the uartUSB.read to recieve input from matrix keypad instead
    struct tm Realtime;
    int strIndex;
    char str[100];
    int stringLength;
    uartUsb.write("\r\nType 4 digits for the current year (YYYY):",46);
    for (strIndex=0; strIndex<4;strIndex++){//enters each entered required digit into struct
            uartUsb.read( &str[strIndex] , 1 ); //reads the input
            uartUsb.write( &str[strIndex] ,1 ); //then prints out in SM, explains double print out
    }
    str[4]='\0';//string is set to null?
    Realtime.tm_year = atoi(str)-1900; //years since 1900, atoi converts string figures in to integers
    uartUsb.write("\r\n",2);

    uartUsb.write("Type 2 digits for the current month (MM):",41); //do not add extra size otherwise junk values
    for( strIndex=0; strIndex<2; strIndex++ ) { //only 2 digits
                uartUsb.read( &str[strIndex] , 1 );
                uartUsb.write( &str[strIndex] ,1 );
        }
        str[2] = '\0';
        Realtime.tm_mon=atoi(str)-1;//why do we -1, jan = 0? must be time calculations
        uartUsb.write("\r\n", 2);

    uartUsb.write( "Type 2 digits for the current day (01-31):", 42 );//day
    for( strIndex=0; strIndex<2; strIndex++ ) {
        uartUsb.read( &str[strIndex] , 1 );
        uartUsb.write( &str[strIndex] ,1 );
        }
            str[2] = '\0';
            Realtime.tm_mday = atoi(str);
            uartUsb.write( "\r\n", 2 );

            uartUsb.write( "Type 2 digits for the current hour (00-23):", 43 ); //hours
            for( strIndex=0; strIndex<2; strIndex++ ) {
                uartUsb.read( &str[strIndex] , 1 );
                uartUsb.write( &str[strIndex] ,1 );
            }
            str[2] = '\0';
            Realtime.tm_hour = atoi(str);
            uartUsb.write( "\r\n", 2 );

            uartUsb.write( "Type 2 digits for the current minutes (00-59):", 46 ); //minutes
            for( strIndex=0; strIndex<2; strIndex++ ) {
                uartUsb.read( &str[strIndex] , 1 );
                uartUsb.write( &str[strIndex] ,1 );
            }
            str[2] = '\0';
            Realtime.tm_min  = atoi(str);
            uartUsb.write( "\r\n", 2 );

            uartUsb.write( "Type 2 digits for the current seconds (00-59):", 46 ); //seconds
            for( strIndex=0; strIndex<2; strIndex++ ) {
                uartUsb.read( &str[strIndex] , 1 );
                uartUsb.write( &str[strIndex] ,1 );
            }
            str[2] = '\0';
            Realtime.tm_sec  = atoi(str);
            uartUsb.write( "\r\n", 2 );

            Realtime.tm_isdst = -1; //daylight savings is in effect
            set_time( mktime( &Realtime ) ); //mktime creates a timestap from a tm structure 
            uartUsb.write( "Date and time has been set\r\n", 28 );
            time_set=true;
// code does work as intended and prints fine
            
}

void timeprint() {
    char str[100];
    time_t epochSeconds;
                epochSeconds = time(NULL);
                sprintf ( str, "Date and Time = %s", ctime(&epochSeconds));
                uartUsb.write( str , strlen(str) );
                uartUsb.write( "\r\n", 2 );
}

//going to use functions from example since these are well made functions that work well with existing code

void systemElementStateUpdate( bool lastState, bool currentState, const char* elementName ){
//this is the log creator and storage function, has 3 different paramters that need to be populated, shown in the eventlogupdate above.

    char eventAndStateStr[EVENT_NAME_MAX_LENGTH] = ""; //declare and initialize eventandstatestr which is 14 (global variable), populated by empty text/"\0"/null same thing

    if ( lastState != currentState ) { //so if the last state doesn't match the current, aka the state of something in the system has changed.
        strcat( eventAndStateStr, elementName ); //copy the name of the element name from the pointers address and append it into the eventandstatestr variable
        //str cat adds/appends strings together look into further
        if ( currentState ) { //so if depending on current state append either 
            strcat( eventAndStateStr, "_ON" ); //on 
        } else { //or 
            strcat( eventAndStateStr, "_OFF" ); //off
        } //just states the state of an alarm

        arrayOfStoredEvents[eventsIndex].seconds = time(NULL); //time from beginning point/ current time
        strcpy( arrayOfStoredEvents[eventsIndex].typeOfEvent,eventAndStateStr ); //copies the string eventandstate string into arrayofstoredvents indexed element
        if ( eventsIndex < EVENT_MAX_STORAGE - 1 ) {//have to subtract 1 since indexing starts at 0
            eventsIndex++; //move onto next indexed struct
        } else { //if greater than set storage limit 
            eventsIndex = 0; //start back from 0
        }

        uartUsb.write( eventAndStateStr , strlen(eventAndStateStr) ); //print the event out 
        uartUsb.write( "\r\n", 2 );
    }
}

void eventLogUpdate() { //checks each necessary variable that needs to be reported on
    systemElementStateUpdate( alarmLastState, alarmState, "ALARM" );
    alarmLastState = alarmState;

    systemElementStateUpdate( gasLastState, !mq2, "GAS_DET" );
    gasLastState = !mq2;

    systemElementStateUpdate( tempLastState, overTempDetectorState, "OVER_TEMP" );
    tempLastState = overTempDetectorState;

    systemElementStateUpdate( ICLastState, incorrectCodeLed, "LED_IC" );
    ICLastState = incorrectCodeLed;

    systemElementStateUpdate( SBLastState, systemBlockedLed, "LED_SB" );
    SBLastState = systemBlockedLed;
}
