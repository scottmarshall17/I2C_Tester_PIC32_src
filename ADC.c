
#include <xc.h>
#include "constants.h"
#include "ADC.h"

void initADC() {
    //Use AN0/RB0 AN2/RB2 AN4/RB4 AN8/RB8 as the ADC pin
    ANSELBbits.ANSB0 = ENABLED;
    ANSELBbits.ANSB2 = ENABLED;
    ANSELBbits.ANSB4 = ENABLED;
    ANSELBbits.ANSB8 = ENABLED;
    TRISBbits.TRISB0 = INPUT;
    TRISBbits.TRISB2 = INPUT;
    TRISBbits.TRISB4 = INPUT;
    TRISBbits.TRISB8 = INPUT;
    
    AD1CON1bits.SIDL = 0;
    AD1CON1bits.FORM = INTEGER;
    AD1CON1bits.SSRC = AUTO;
    AD1CON1bits.CLRASAM = 0;
    AD1CON1bits.ASAM = 1;
    AD1CON2bits.VCFG = 0x00;
    AD1CON2bits.OFFCAL = 0;
    AD1CON2bits.CSCNA = SCAN;
    AD1CON2bits.BUFM = DOUBLE_BUFFER;
    AD1CON2bits.SMPI = 3;   //because we have 4 inputs to be scanned
    AD1CON2bits.ALTS = 0;
    AD1CON3bits.ADRC = 0;
    AD1CON3bits.SAMC = 0xF;
    AD1CON3bits.ADCS = 1;
    AD1CHSbits.CH0NA = 0;
    //AD1CHSbits.CH0SA = 0;
    
    AD1CSSLbits.CSSL0 = 1;  //Right Sensor
    AD1CSSLbits.CSSL2 = 1;  //Front Sensor
    AD1CSSLbits.CSSL4 = 1;  //Left Sensor
    AD1CSSLbits.CSSL8 = 1;  //Middle Sensor
    
    IFS0bits.AD1IF = FLAG_DOWN;
    //IEC0bits.AD1IE = 1;   //not sure if this needs to be on or not
    
    AD1CON1bits.ON = ENABLED;
    
    
    return;
}

char readSensors(void){ //returns char with format 0b0000[Right][Top][Left][Middle]
    char result = 0;
    int an0, an2, an4, an8;
    
    while(!IFS0bits.AD1IF == 1);    //wait for the interrupt flag
    AD1CON1bits.ASAM = 0;
    
    if(AD1CON2bits.BUFS == 1) {
        an0 = ADC1BUF0;
        an2 = ADC1BUF1;
        an4 = ADC1BUF2;
        an8 = ADC1BUF3;
    }
    else {
        an0 = ADC1BUF8;
        an2 = ADC1BUF9;
        an4 = ADC1BUFA;
        an8 = ADC1BUFB;
    }
    
    if(an0 < THRESHOLD1) {
        result = result | 1;
    }
    else {
        result = 0;
    }
    if(an2 < THRESHOLD2) {
        result = (result << 1) | 1;
    }
    else {
        result = result << 1;
    }
    if(an4 < THRESHOLD3) {
        result = (result << 1) | 1;
    }
    else {
        result = result << 1;
    }
    if(an8 < THRESHOLD4) {
        result = (result << 1) | 1;
    }
    else {
        result = result << 1;
    }
    
    AD1CON1bits.ASAM = ENABLED;
    IFS0bits.AD1IF = FLAG_DOWN;
    
    return result;
}

int testSensor(char bitArray) {
    int result = 0;
    int an0, an2, an4, an8;
    
    while(!IFS0bits.AD1IF == 1);    //wait for the interrupt flag
    AD1CON1bits.ON = DISABLED;
    AD1CON1bits.ASAM = 0;
    
    if(AD1CON2bits.BUFS == 1) {
        an0 = ADC1BUF0;
        an2 = ADC1BUF1;
        an4 = ADC1BUF2;
        an8 = ADC1BUF3;
    }
    else {
        an0 = ADC1BUF8;
        an2 = ADC1BUF9;
        an4 = ADC1BUFA;
        an8 = ADC1BUFB;
    }
    
    AD1CON1bits.ASAM = ENABLED;
    AD1CON1bits.ON = ENABLED;
    IFS0bits.AD1IF = FLAG_DOWN;
    
    if(((bitArray >> 3) & 1) == 1) {
        result = an0;
    }
    else if(((bitArray >> 2) & 1) == 1) {
        result = an2;
    }
    else if(((bitArray >> 1) & 1) == 1) {
        result = an4;
    }
    else if(((bitArray) & 1) == 1) {
        result = an8;
    }
    else {
        result = -1;
    }
    
    return result;
}
//returns char with format 0b0000[Right][Top][Left][Middle]

boolean frontTriggered(char sensors){
    boolean result = true;
    result = (sensors >> 2) & 1;
    return result;
}
boolean rightTriggered(char sensors){
    boolean result = true;
    result = (sensors >> 3) & 1;
    return result;
}
boolean leftTriggered(char sensors){
    boolean result = true;
    result = (sensors >> 1) & 1;
    return result;
}
boolean middleTriggered(char sensors){
    boolean result = true;
    result = sensors & 1;
    return result;
}
