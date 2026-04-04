//week 4 header file containing functions declerations
// mainly for sensor functions, global variable definitions and initialization 
//ifndef prevents the header file from being defined multiple times causing definition errors
#ifndef Week_4_h 
#define Week_4_h

//=====[Libraries]=============================================================
//had to include libraries to ensure that header file knew what digital out and pins are
#include "mbed.h"
#include "arm_book_lib.h"


//=====[Declaration and initialization of public global objects]=============
extern DigitalOut sirenPin; //put external decleration in here to prevent undecleration or re-declerations from happening

extern DigitalOut alarmLed; //had to put external dec leds to use in week 5 header 
extern DigitalOut incorrectCodeLed;
extern DigitalOut systemBlockedLed;

extern UnbufferedSerial uartUsb;

extern DigitalIn mq2;
//=====[Declaration and initialization of public global variables]=============
extern bool alarmState;
extern Timer Reading_Timer;
extern bool alarmState;
extern bool gasDetectorState;
extern bool overTempDetectorState;
extern float lm35TempC;
/*the header files for now only consist of prototyping of functions all other declerations and
all other declerations, pin assignments etc are to go in to the cpp file*/
//=====[Declarations (prototypes) of public functions]=========================
void TempRead();
void TempInC();
void GasRead();
void Potentiometer_Read();
void SensorValPrint();
void ThresholdSet();
void AlarmCheck();
void buzzer();
void LM35ReadingsArrayInit();
void OutputsInit();
//=====[#include guards - end]=================================================

#endif
