#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn buttons[6]={DigitalIn(D2),DigitalIn(D3),DigitalIn(D4),DigitalIn(D5),DigitalIn(D6),DigitalIn(D7)};
DigitalIn enterButton(BUTTON1); //button input initialization


DigitalOut cor_LED(LED1);
DigitalOut incor_LED(LED3);

    int passcode[4]={2,4,5,0}; //correct passcode declare
    int user_input[4]; //declaring user input array
    int digit=0;

void flash_incor() {
    for(int f=0;f<6;f++){
    incor_LED=!incor_LED;
    ThisThread::sleep_for(200ms);
    }
}

void flash_cor() { //simple flashing 
    for(int f=0;f<6;f++){
    cor_LED=!cor_LED;
    ThisThread::sleep_for(200ms);
    }
}
void inp_flash() { //created inp_flash to help with debugging and to show when inputs are entered
    for(int f=0;f<2;f++){
    cor_LED=!cor_LED;
    ThisThread::sleep_for(200ms);
}

}

bool input_read(int& input) { //address ref or pointer used here? can use both but reference is better in general
//so learn how to use references for the future
    for (int i = 0; i < 6; i++) {
        if (buttons[i] == 1) {
            inp_flash();
            ThisThread::sleep_for(200ms); // debounce
            input=i;
            
            return true;
        }
    }
    return false;  // no button pressed
}

bool check(){
        for (int j=0;j<4;j++) {
            if (passcode[j]!=user_input[j]){//passcode is 0251
            return false;
            }
    }
    return true;
}
