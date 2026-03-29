//=====[Libraries]=============================================================  //keep working on header files 
#include "Week_4.h"
#include "mbed.h"
#include "arm_book_lib.h"
#include "Week_5.h"
// #include "Week_4.h" //just double check if we need that

//=====[Defines]===============================================================


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
char keyPressed[NUMBER_OF_KEYS] = { '0', '0', '0', '0' }; //stores user input
int accumulatedTimeAlarm = 0; //how many times has incorrect passcode been set off?


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
        if( accumulatedDebounceKeypadTime >= KEYPAD_DEBOUNCE)//checks to see if debounce time has elapsed
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
    char str[50]; int StringLength;//might not work but unsure need to test
    //add if numberofincorrectcodes<x for system block
    char KeyReleased = KeypadUpdate(); //the key released means the key pressed
    if (KeyReleased != '\0' && KeyReleased !='#' && KeyReleased != '*'){ //key isn't null and # (enter), can't be or otherwise it would always be true in idle

        if (KeypadCodeIndex >= NUMBER_OF_KEYS) { //if greater than size of array
            KeypadCodeIndex = 0; //go back to start of passcode array
        }
        keyPressed[KeypadCodeIndex] = KeyReleased;
         KeypadCodeIndex++;//increase index by 1 and populate next member

    }
    if (KeyReleased ==  '#'){ //enter route
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
                    delay(30000); //if the code is correct freeze system for 30 seconds
            } else {
                sprintf(str,"\rIncorrect code\n");
                    StringLength=strlen(str);
                for (int i=0;i<6;i++) { //flahses incor led 3 times if wrong code is displayed, delay might cause bugs but not guaranteed
                    incorrectCodeLed =! incorrectCodeLed;
                    delay(500);
                }
                numberOfIncorrectCodes++;
                        
                    }
                }
            }
    if (KeyReleased == '*') { //clear route
        codewipe();//wipes entered code
    }
            //don't need system block yet
}
    


bool areEqual(){
    int i;

    for (i = 0; i < NUMBER_OF_KEYS; i++) {
        if (codeSequence[i] != keyPressed[i]) {
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
