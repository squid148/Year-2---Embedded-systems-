//W6_T2
//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"
#include "Week_4.h" 
#include "Week_5.h"
#include "display.h"
// header file since all sensor related functions are no longer highlighted
// === [definitions have been put into headers to categories them and clean up main]==

//=====[decleration and initialization of public global objects moved to respective header files]
//code is 2943

int main() {  
    //could make user input the date at the start of the system
     InputsInit(); //initializes keypad and temp array
    OutputsInit(); //sets siren pin to off
    Reading_Timer.start(); //fix reading next, try extern declaring in week_4.h to fix it// fixed
    while (true) {
        
        alarmdeactivate(); 
        if (!time_set){
        timesetter();// function asks user to set time once at the begginning of the programm before continuing
        timeprint();//prints time out, can be called for 
        }
        else{
        eventLogUpdate(); 
        }
        if (Reading_Timer.elapsed_time()>=1000ms){
            sensors();
            AlarmCheck();//check to see if any alarms are on and if so print according message
            LCDReportCheck();
            LCDAlarmTrigMessage();
            //maybe turn all of the sensor reading and alarm setter into a function to clean up previous code

        if (alarmState){
            buzzer();// enables buzzer in alarm conditions and states reason
            codeprint(); //prints entered code
        }
        else {
        sirenPin=OFF; /*sirepin is now ackowledged by main, have to use external declare on some variables
            so compiler knows that they exist */
        //SensorValPrint(); //comment out when running
        
        }
        ElapsedLoops++;
        //add in line add 1 to a counter and if counter reaches 60, print alarm states
        Reading_Timer.reset(); //has to be inside loop otherwise it will constantly reset meaning nothing is run
        
        }
        
    }
    delay(TIME_INCREMENT_MS);//loop increment setter
} 
