// ******************************************************************************************* //
//  This comment is a test for the github repository. test again.
// File:         lab2p1.c
// Date:         
// Authors:      
//
// Description: Part 1 for lab 1
// ******************************************************************************************* //

#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "constants.h"
#include "leds.h"
#include "interrupt.h"
#include "switch.h"
#include "button.h"
#include "timer.h"
#include "lcd.h"
#include "keypad.h"
#include "ADC.h"
#include "PWM.h"
#include "control.h"


typedef enum state_enum {
    INIT, TOP, RIGHT, MIDDLE, LEFT, SET_DIRECTION, PRINT_LCD, FORWARD, RESUME
} state_t;

volatile state_t myState;
volatile int read;
volatile int read_reset;    /*integer to store value of the reset button*/
volatile char timer_flag;   /*The timer flag increments every 10ms*/
/* Please note that the configuration file has changed from lab 0.
 * the oscillator is now of a different frequency.
 */
int main(void)
{
    state_t direction = FORWARD;
    int voltageADC = 0;
    int lastVoltage = 0;
    char charToWrite = 0;
    char numberToPrint[5] = {' ', ' ',  ' ',  ' ',  '\0'};
    char sensors = 0b00000001;
    SYSTEMConfigPerformance(10000000);
    enableInterrupts();
    myState = MIDDLE;
    initButton();
    initLEDs();
    initTimers();
    initLCD();
    initKeypad();
    initPWM();
    initADC();
    
    //format 0b0000[Right][Top][Left][Middle]
    
    while(1)
    {
        switch(myState) {
            case INIT:
                sensors = 0b00000100;
                myState = TOP;
                break;
            case RIGHT:
                voltageADC = testSensor(0b00001000);
                if(myState != SET_DIRECTION) {
                    myState = PRINT_LCD;
                }
                break;
            case TOP:
                voltageADC = testSensor(0b00000100);
                if(myState != SET_DIRECTION) {
                    myState = PRINT_LCD;
                }
                break;
            case LEFT:
                voltageADC = testSensor(0b00000010);
                if(myState != SET_DIRECTION) {
                    myState = PRINT_LCD;
                }
                break;
            case MIDDLE:
                voltageADC = testSensor(0b00000001);
                if(myState != SET_DIRECTION) {
                    myState = PRINT_LCD;
                }
                break;
            case SET_DIRECTION:
                switch(sensors) {
                        case 0b00001000:
                            sensors = 0b00000100;
                            myState = TOP;
                            break;
                        case 0b00000100:
                            sensors = 0b00000010;
                            myState = LEFT;
                            break;
                        case 0b00000010:
                            sensors = 0b00000001;
                            myState = MIDDLE;
                            break;
                        case 0b00000001:
                            sensors = 0b00001000;
                            myState = RIGHT;
                            break;
                        default:
                            myState = INIT;
                            break;
                }
                
                break;
            case RESUME:
                if(myState != SET_DIRECTION) {
                    switch(sensors) {
                        case 0b00001000:
                            myState = RIGHT;
                            break;
                        case 0b00000100:
                            myState = TOP;
                            break;
                        case 0b00000010:
                            myState = LEFT;
                            break;
                        case 0b00000001:
                            myState = MIDDLE;
                            break;
                        default:
                            myState = INIT;
                            break;
                    }
                }
                break;
            case PRINT_LCD:
                itoa(numberToPrint, voltageADC, 10);
                clearLCD();
                delayUs(10000);
                moveCursorLCD(0, 5);
                printStringLCD(numberToPrint);
                delayUs(30000);
                if(myState != SET_DIRECTION) {
                    myState = RESUME;
                }
                break;
        }
    }
    
    return 0;
}

void __ISR(_TIMER_1_VECTOR, IPL7SRS) timer1Handler(void){
    IFS0bits.T1IF = FLAG_DOWN;
    TMR1 = 0;
}

/*
** The change notification interrupt handles the start/stop button and the 
** reset button logic, hence its length. There are really only two switch statements
** in the ISR, so it really doesn't have to make many decisions that would 
** bloat the ISR.
*/

void __ISR(_CHANGE_NOTICE_VECTOR, IPL7SRS) _CNInterrupt(void){
    //TODO: Implement the interrupt to capture the press of the button
    PORTD;
    IFS1bits.CNDIF = FLAG_DOWN;
    IFS0bits.T4IF = 0;      //lower timer 4 flag for delay
    TMR4 = 0;  
    PR4 = 15000;//manually clear timer 4 register
    T4CONbits.ON = 1;       //turn timer 4 on.
    while(IFS0bits.T4IF != 1){};    //5ms delay
    T4CONbits.ON = 0;       //turn off timer 4
    IFS0bits.T4IF = 0;
    read = PORTD;
    read = PORTDbits.RD7;
    if(read == 0){
        switch(myState) {
            default:
                myState = SET_DIRECTION;
                break;
        }
    } else {
        myState = myState;
    }
}
