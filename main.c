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
#include "LSM303CTypes.h"
#include "I2C.h"


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
    int i = 0;
    char I2Cdata = 0;
    int magData = 0;
    int magDataOld = 0;
    char I2Ctemp = 0;
    int sum = 0;
    int num = 0;
    int lastVoltage = 0;
    char charToWrite = 0;
    char numberToPrint[7] = {' ', ' ', ' ', ' ',  ' ',  ' ',  '\0'};
    char sensors = 0b00000001;
    SYSTEMConfigPerformance(10000000);
    enableInterrupts();
    myState = INIT;
    initButton();
    initLEDs();
    initTimers();
    initLCD();
    initKeypad();
    initPWM();
    initADC();
    initI2C();
    startBreakout();
    //format 0b0000[Right][Top][Left][Middle]
    
    while(1)
    {
        switch(myState) {
            case INIT:
                
                /*
                if(I2CReadRegister(0x0F, &I2Cdata, ACC_I2C_ADDR) == 1) {
                    myState = PRINT_LCD;
                    I2Ctemp = I2Cdata;
                    magData = I2Ctemp;
                    /*
                    I2Cdata = 0b01100000;
                    if(I2CWriteBytes(0x20, &I2Cdata, 1, 0x1E) == 1) {
                        if(I2CReadRegister(0x20, &I2Ctemp, 0x1E) == 1) {
                            myState = PRINT_LCD;
                        }
                    }
                     //                
                }
                */
                sum = 0;
                for(i = 0; i < 2; ++i) {
                    if(ACC_readZ(&magData) == 1) {
                        if(1) {
                            magDataOld = magData;
                            sum = sum + magData;
                            myState = PRINT_LCD;
                        }
                        else {
                            myState = INIT;
                        }
                    } 
                }
                magData = sum/2;
                magData = magData >> 8;
                myState = PRINT_LCD;
                break;
            case PRINT_LCD:
                num = ((int)I2Ctemp)&(0x000000FF);
                itoa(numberToPrint, magData, 10);
                clearLCD();
                delayUs(10000);
                moveCursorLCD(0, 5);
                printStringLCD(numberToPrint);
                delayUs(30000);
                if(myState != SET_DIRECTION) {
                    myState = INIT;
                }
                myState = INIT;
                break;
            case RESUME:
                while(1);
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
