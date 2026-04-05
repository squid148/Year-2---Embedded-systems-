//week 5 task header
//mainly contains matrixpad and passcode enter definitions, structs, variables
//put all this in cpp file and link back to header
#ifndef WEEK_5_H //prevents erros later on
#define WEEK_5_H


#include "mbed.h"
#include "arm_book_lib.h"
#include "Week_4.h"

//=====[Declaration and initialization of public global variables]=============

#define NUMBER_OF_KEYS                           5 //size of password array
#define DEBOUNCE_KEY_TIME_MS                    40 //debounce timer to prevent debounce errors
#define KEYPAD_NUMBER_OF_ROWS                    4 //num of rows for keypad
#define KEYPAD_NUMBER_OF_COLS                    4 // num of columns
#define EVENT_MAX_STORAGE                      100 //used for a array of structs to access specific data stored in each one
#define EVENT_NAME_MAX_LENGTH                   14 //how long can a event name be
#define TIME_INCREMENT_MS                       2000 //loop increment, don't put ms on end causes bugs

//=====[Declaration of public data types]======================================




//=====[decleration and initialization of public global objects ]=============



//=====[Declaration and initialization of public global variables]=============

extern char keyPressed[NUMBER_OF_KEYS];
extern bool time_set;

extern bool alarmLastState; //decided to move last alarm to be global vairables so they can be used else 
extern bool gasLastState; //eg display.cpp to show when the alarm state has changed to on to give a warning
extern bool tempLastState;
extern bool ICLastState;
extern bool SBLastState;

//event index variables

//=====[function prototyping]==============================================================
void InputsInit();
void KeypadInit();
void LM35ReadingsArrayInit(); //have to prototype in header to fix it
char KeypadUpdate();
char KeypadScan();
void alarmdeactivate();
bool areEqual();
void sensors();
void codeprint();
void codewipe();
void timesetter();
void timeprint();
void systemElementStateUpdate( bool lastState, bool currentState, const char* elementName );
void eventLogUpdate();
bool LCDReportRequest();
//=====[functions]==============================================================



#endif
