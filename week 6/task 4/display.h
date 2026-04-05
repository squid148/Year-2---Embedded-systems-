//=====[#include guards - begin]===============================================

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

//=====[Declaration of public defines]=========================================

//=====[Declaration and initialization of public global variables]=============
extern int ElapsedLoops;

extern bool LCDtempLastState; //decided to add new last states since this may be causing issues using week 5 ones
extern bool LCDgasLastState;
//=====[Declaration of public data types]======================================

typedef enum {
     DISPLAY_CONNECTION_GPIO_4BITS,
     DISPLAY_CONNECTION_GPIO_8BITS,
     DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER,
} displayConnection_t;

typedef struct {
   displayConnection_t connection;                                                 
} display_t;

//=====[Declarations (prototypes) of public functions]=========================

void displayInit( displayConnection_t connection );
 
void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY );

void displayStringWrite( const char * str );

void displaywipe();

void LCDReport();

void LCDReportCheck();

void LCDAlarmTrigMessage();

void StartupDeactivateCode();

void LCDStartupPrompt();
//=====[#include guards - end]=================================================

#endif // _DISPLAY_H_
