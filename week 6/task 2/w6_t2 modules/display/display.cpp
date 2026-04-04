//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "display.h"
#include "Week_5.h"
#include "Week_4.h"
//=====[Declaration of private defines]========================================

#define DISPLAY_IR_CLEAR_DISPLAY   0b00000001 //good idea to use defines to store bits for certain commands so you don't have to type it every time
#define DISPLAY_IR_ENTRY_MODE_SET  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL 0b00001000
#define DISPLAY_IR_FUNCTION_SET    0b00100000
#define DISPLAY_IR_SET_DDRAM_ADDR  0b10000000

#define DISPLAY_IR_ENTRY_MODE_SET_INCREMENT 0b00000010
#define DISPLAY_IR_ENTRY_MODE_SET_DECREMENT 0b00000000
#define DISPLAY_IR_ENTRY_MODE_SET_SHIFT     0b00000001
#define DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT  0b00000000

#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF 0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_ON   0b00000010
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF  0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_ON    0b00000001
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF   0b00000000

#define DISPLAY_IR_FUNCTION_SET_8BITS    0b00010000
#define DISPLAY_IR_FUNCTION_SET_4BITS    0b00000000
#define DISPLAY_IR_FUNCTION_SET_2LINES   0b00001000
#define DISPLAY_IR_FUNCTION_SET_1LINE    0b00000000
#define DISPLAY_IR_FUNCTION_SET_5x10DOTS 0b00000100
#define DISPLAY_IR_FUNCTION_SET_5x8DOTS  0b00000000

#define DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS 0 //is this the start of the next line in the lcd display
#define DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS 64
#define DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS 20
#define DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS 84

#define DISPLAY_RS_INSTRUCTION 0 //storing lcd command and data mode 
#define DISPLAY_RS_DATA        1

#define DISPLAY_RW_WRITE 0 //read and write modes for rw pin
#define DISPLAY_RW_READ  1

#define DISPLAY_PIN_RS  4 
#define DISPLAY_PIN_RW  5
#define DISPLAY_PIN_EN  6
#define DISPLAY_PIN_D0  7  
#define DISPLAY_PIN_D1  8  
#define DISPLAY_PIN_D2  9  
#define DISPLAY_PIN_D3 10
#define DISPLAY_PIN_D4 11
#define DISPLAY_PIN_D5 12 
#define DISPLAY_PIN_D6 13 
#define DISPLAY_PIN_D7 14 

#define DISPLAY_PIN_A_PCF8574 3

#define I2C1_SDA PB_9 //I2C bus pins defined as PB_8 and 9
#define I2C1_SCL PB_8

#define PCF8574_I2C_BUS_8BIT_WRITE_ADDRESS 78

#define ReportTimer    60//mimics 1 whole minute passing and can be used for checking when 1 minute has passed
//=====[Declaration of private data types]=====================================

typedef struct{ //struct to store bits, lcd address 
    int address;
    char data; //what is data for
    bool displayPinRs;
    bool displayPinRw; 
    bool displayPinEn;
    bool displayPinA;
    bool displayPinD4; //only 4 pins so data must be sent in 2 sets 
    bool displayPinD5;
    bool displayPinD6;
    bool displayPinD7;
} pcf8574_t;

//=====[Declaration and initialization of public global objects]===============

DigitalOut displayD0( D0 ); //declare dispaly pins for bits
DigitalOut displayD1( D1 );
DigitalOut displayD2( D2 );
DigitalOut displayD3( D3 );
DigitalOut displayD4( D4 );
DigitalOut displayD5( D5 );
DigitalOut displayD6( D6 );
DigitalOut displayD7( D7 );
DigitalOut displayRs( D8 );
DigitalOut displayEn( D9 );

I2C i2cPcf8574( I2C1_SDA, I2C1_SCL ); //pins 8 and 9 are used for the I2C bus communications

//=====[Declaration of external public global variables]=======================
int ElapsedLoops=99;
int minutes;
bool LCDtempLastState = OFF;
bool LCDgasLastState  = OFF;
//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

static display_t display;
static pcf8574_t pcf8574;
static bool initial8BitCommunicationIsCompleted;

//=====[Declarations (prototypes) of private functions]========================

static void displayPinWrite( uint8_t pinName, int value ); //prototyping privetly since don't need to be brought into other files just work internally to support public funcions declared in header
static void displayDataBusWrite( uint8_t dataByte );
static void displayCodeWrite( bool type, uint8_t dataBus );

//=====[Implementations of public functions]===================================

void displayInit( displayConnection_t connection )
{
    display.connection = connection;
    
    if( display.connection == DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER) {
        pcf8574.address = PCF8574_I2C_BUS_8BIT_WRITE_ADDRESS;
        pcf8574.data = 0b00000000; //the 0b doesn't mean 0 and break for certain pins it means binary apperently
        i2cPcf8574.frequency(100000);
        displayPinWrite( DISPLAY_PIN_A_PCF8574,  ON );
    } 
    
    initial8BitCommunicationIsCompleted = false;    

    delay( 50 );
    
    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_FUNCTION_SET | 
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    delay( 5 );
            
    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_FUNCTION_SET | 
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    delay( 1 ); 

    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_FUNCTION_SET | 
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    delay( 1 );  

    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_FUNCTION_SET | 
                              DISPLAY_IR_FUNCTION_SET_8BITS | 
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            delay( 1 );         
        break;
        
        case DISPLAY_CONNECTION_GPIO_4BITS:
        case DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_FUNCTION_SET | 
                              DISPLAY_IR_FUNCTION_SET_4BITS );
            delay( 1 );  

            initial8BitCommunicationIsCompleted = true;  

            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_FUNCTION_SET | 
                              DISPLAY_IR_FUNCTION_SET_4BITS | 
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            delay( 1 );                                      
        break;
    }

    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF |      
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |       
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );       
    delay( 1 );          

    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_CLEAR_DISPLAY );       
    delay( 1 ); 

    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_ENTRY_MODE_SET |
                      DISPLAY_IR_ENTRY_MODE_SET_INCREMENT |       
                      DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT );                  
    delay( 1 );           

    displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON |      
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |    
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );    
    delay( 1 );  
}

void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY ) //takes parametrs for x and y coordiantes of written data
{    
    switch( charPositionY ) {
        case 0:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay( 1 );         
        break;
       
        case 1:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay( 1 );         
        break;
       
        case 2:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay( 1 );         
        break;

        case 3:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION, 
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            delay( 1 );         
        break;
    }
}

void displayStringWrite( const char * str ) //most important function that allows for a string to be written straight onto the display, use later 
{
    while (*str) {
        displayCodeWrite(DISPLAY_RS_DATA, *str++);
    }
}

//=====[Implementations of private functions]==================================

static void displayCodeWrite( bool type, uint8_t dataBus ) //writes rs, so either sets it into command or data mode
{
    if ( type == DISPLAY_RS_INSTRUCTION ) //so if they type paramter == 0 (display_rs_instruction value for insturctions) then,
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_INSTRUCTION); // run it in instruction mode if type == 0 or
        else 
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_DATA); //run it in data and write mode if type == 1, or any other character (just to set a deafult)
    displayPinWrite( DISPLAY_PIN_RW, DISPLAY_RW_WRITE ); //and write the data into the display when in data mode
    displayDataBusWrite( dataBus ); //databus manages enable pin and allows data to be transfered to the LCD screen
}

static void displayPinWrite( uint8_t pinName, int value )
{
    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS: //in 8 bit mode
            switch( pinName ) {
                case DISPLAY_PIN_D0: displayD0 = value;   break; //D0-D7 are your data pins and you enter the required ascii number using bit wise input
                case DISPLAY_PIN_D1: displayD1 = value;   break; //eg I want to write the letter B, so I set my bits to 01000010, which is 66 in the ascii table
                case DISPLAY_PIN_D2: displayD2 = value;   break; //and then the lcd dispaly will process and output B in whatever position I have specfied after the enable pulse has finished
                case DISPLAY_PIN_D3: displayD3 = value;   break;
                case DISPLAY_PIN_D4: displayD4 = value;   break;
                case DISPLAY_PIN_D5: displayD5 = value;   break;
                case DISPLAY_PIN_D6: displayD6 = value;   break;
                case DISPLAY_PIN_D7: displayD7 = value;   break;
                case DISPLAY_PIN_RS: displayRs = value;   break; //RS stand for register select and depending and it determines if the lcd is recieving a command (low) or data (high)
                case DISPLAY_PIN_EN: displayEn = value;   break; //en is the latch enable so a pulse has to be sent every time you want a command to be read and latched
                case DISPLAY_PIN_RW: break; //rw pin sets it to read or write mode but why does it cause break, reason: the code is always in write mode so no need for value change
                default: break;
            }
            break;
        case DISPLAY_CONNECTION_GPIO_4BITS:
            switch( pinName ) {
                case DISPLAY_PIN_D4: displayD4 = value;   break;
                case DISPLAY_PIN_D5: displayD5 = value;   break;
                case DISPLAY_PIN_D6: displayD6 = value;   break;
                case DISPLAY_PIN_D7: displayD7 = value;   break;
                case DISPLAY_PIN_RS: displayRs = value;   break;
                case DISPLAY_PIN_EN: displayEn = value;   break;
                case DISPLAY_PIN_RW: break; 
                default: break;
            }
            break;
        case DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER:
           if ( value ) {
                switch( pinName ) { //turn on all pins
                    case DISPLAY_PIN_D4: pcf8574.displayPinD4 = ON; break;
                    case DISPLAY_PIN_D5: pcf8574.displayPinD5 = ON; break;
                    case DISPLAY_PIN_D6: pcf8574.displayPinD6 = ON; break;
                    case DISPLAY_PIN_D7: pcf8574.displayPinD7 = ON; break;
                    case DISPLAY_PIN_RS: pcf8574.displayPinRs = ON; break;
                    case DISPLAY_PIN_EN: pcf8574.displayPinEn = ON; break;
                    case DISPLAY_PIN_RW: pcf8574.displayPinRw = ON; break;
                    case DISPLAY_PIN_A_PCF8574: pcf8574.displayPinA = ON; break;
                    default: break;
                }
            }
            else {
                switch( pinName ) { //or turn of all pins 
                    case DISPLAY_PIN_D4: pcf8574.displayPinD4 = OFF; break;
                    case DISPLAY_PIN_D5: pcf8574.displayPinD5 = OFF; break;
                    case DISPLAY_PIN_D6: pcf8574.displayPinD6 = OFF; break;
                    case DISPLAY_PIN_D7: pcf8574.displayPinD7 = OFF; break;
                    case DISPLAY_PIN_RS: pcf8574.displayPinRs = OFF; break;
                    case DISPLAY_PIN_EN: pcf8574.displayPinEn = OFF; break;
                    case DISPLAY_PIN_RW: pcf8574.displayPinRw = OFF; break;
                    case DISPLAY_PIN_A_PCF8574: pcf8574.displayPinA = OFF; break;
                    default: break;
                }
            }     
            pcf8574.data = 0b00000000; //assigning data values
            if ( pcf8574.displayPinRs ) pcf8574.data |= 0b00000001; //one by one the pins field of pcf8574 is evaluated
            if ( pcf8574.displayPinRw ) pcf8574.data |= 0b00000010; //if on the correspodning bit of pcf8574.data is turned on
            if ( pcf8574.displayPinEn ) pcf8574.data |= 0b00000100; //so if pcf8574.displayPinEn is true the pcf8574.data is set to 0b00000100
            if ( pcf8574.displayPinA  ) pcf8574.data |= 0b00001000; //so we are setting each bit individually in command mode to clear the screen
            if ( pcf8574.displayPinD4 ) pcf8574.data |= 0b00010000; 
            if ( pcf8574.displayPinD5 ) pcf8574.data |= 0b00100000; 
            if ( pcf8574.displayPinD6 ) pcf8574.data |= 0b01000000; 
            if ( pcf8574.displayPinD7 ) pcf8574.data |= 0b10000000; 
            i2cPcf8574.write( pcf8574.address, &pcf8574.data, 1);//the corresponding value is transferred using this code line
            break;    //the first paramater is the address, then the data to be transfered (data stored in data address, and the amount of bytes to be transffered)
    }
}

static void displayDataBusWrite( uint8_t dataBus )
{
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b10000000 );
    displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b01000000 );
    displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00100000 );
    displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00010000 );
    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayPinWrite( DISPLAY_PIN_D3, dataBus & 0b00001000 );
            displayPinWrite( DISPLAY_PIN_D2, dataBus & 0b00000100 );  
            displayPinWrite( DISPLAY_PIN_D1, dataBus & 0b00000010 );      
            displayPinWrite( DISPLAY_PIN_D0, dataBus & 0b00000001 );
        break; 
              
        case DISPLAY_CONNECTION_GPIO_4BITS:
        case DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER: //was does expander do then, is this a 4 bit or 8 bit system or both
            if ( initial8BitCommunicationIsCompleted == true) {
                displayPinWrite( DISPLAY_PIN_EN, ON );         
                delay( 1 );
                displayPinWrite( DISPLAY_PIN_EN, OFF );              
                delay( 1 );        
                displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b00001000 );
                displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b00000100 );  
                displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00000010 );      
                displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00000001 );                
            }
        break;
    
    }
    displayPinWrite( DISPLAY_PIN_EN, ON );              
    delay( 1 );
    displayPinWrite( DISPLAY_PIN_EN, OFF );  
    delay( 1 );                   
}

void displaywipe() {
    displayCodeWrite(0, DISPLAY_IR_CLEAR_DISPLAY);
}

void LCDReport() {
    char str[100]; int StringLength;
    displaywipe();
            uartUsb.write(str, strlen(str));
            displayCharPositionWrite(0, 0); //set cursor to start of LCD dispaly / top left
            displayStringWrite("Over Temp state: "); //write into LCD
                if (overTempDetectorState==HIGH) { //check Otmemp state
                    displayStringWrite("On "); //If high print On
                }
                else {displayStringWrite("Off");} //else print off

            //now for the gas detector section
            displayCharPositionWrite(0, 1); //move down to the next line
            displayStringWrite("Gas  state: ");
                if (gasDetectorState==HIGH){ 
                displayStringWrite("On");
                }
                else {displayStringWrite("Off");}
                codewipe();
}

void LCDReportCheck() {
    if (ElapsedLoops>=ReportTimer){
        
        LCDReport();
        ElapsedLoops=0;
    }
}

void LCDAlarmTrigMessage() {

if (overTempDetectorState != LCDtempLastState || gasDetectorState != LCDgasLastState){//if the alarm has changed

    if (overTempDetectorState == HIGH && gasDetectorState == HIGH){ //temp and gas alarm are on
    
    displayCharPositionWrite(0, 2);
    displayStringWrite("Over temp limit!!!!!");
    displayCharPositionWrite(0, 3);
    displayStringWrite("Over gas limit!!!!!");
        }
    else if (overTempDetectorState == HIGH){ //route where only temp alarm is on
        
        displayCharPositionWrite(0, 2);
        displayStringWrite("Over temp limit!!!!!");
        displayCharPositionWrite(0, 3);
        displayStringWrite("                    ");
        }
    else if(gasDetectorState == HIGH){ //only gas is on
        displayCharPositionWrite(0, 2);
        displayStringWrite("Over gas limit!!!!!");
        displayCharPositionWrite(0, 3);
        displayStringWrite("                    ");
    }
    else {displayCharPositionWrite(0, 2);
            displayStringWrite("                    "); //ended up using spaces just to clear the lines, 
            displayCharPositionWrite(0, 3);
            displayStringWrite("                    ");//couldn't think of another way without making it too complicated
            }

    LCDtempLastState = overTempDetectorState; //have to reassign states otherwise it will constantly give an alarm
    LCDgasLastState = gasDetectorState;

    }
} 
