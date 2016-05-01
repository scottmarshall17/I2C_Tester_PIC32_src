
#include <xc.h>
#include "I2C.h"
#include "constants.h"


void initI2C(void) {
    I2C2CONbits.ON = DISABLED;
    I2C2BRG = 0x30;     //100kHz mode
    I2C2CONbits.SIDL = DISABLED;
    I2C2CONbits.SCLREL = ENABLED;
    I2C2CONbits.STRICT = DISABLED;
    I2C2CONbits.A10M = DISABLED; //7-bit address mode
    I2C2CONbits.DISSLW = DISABLED;
    I2C2CONbits.SMEN = DISABLED;
    I2C2CONbits.GCEN = DISABLED;
    I2C2CONbits.ACKDT = DISABLED; //send NACK when requested
    I2C2CONbits.ON = ENABLED;
    return;
}
void I2CIdle(void) {
    char temp = 250;
    //wait for I2C bus to be inactive
    while(I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RCEN || I2C2CONbits.RSEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT || temp--);
    return;
}
int I2CStart(void) {
    int result = 1;
    I2CIdle();
    I2C2CONbits.SEN = ENABLED;
    
    if(I2C2STATbits.BCL) {
        result = 0;
    }
    else {
        result = 1;
        I2CIdle();
    }
    return result;
}
int I2CStop(void) {
    int result = 1;
    char temp = 0;
    I2CIdle();
    I2C2CONbits.PEN = ENABLED;
    
    while(I2C2CONbits.PEN) {
        if(temp++ > 50) {
            break;
        }
    }
    
    I2C2CONbits.RCEN = DISABLED;
    I2C2STATbits.IWCOL = DISABLED;
    I2C2STATbits.BCL = DISABLED;
    
    I2CIdle();
    
    return result;
}
int I2CWriteByte(char slaveAddr, char dataAddr, char data) {
    int result = 1;
    
    return result;
}

int I2CSendByte(char data) {
    int result = 1;
    I2C2TRN = data;
    
    if(I2C2STATbits.IWCOL == 1) {
        result = 0;
    }
    else {
        while(I2C2STATbits.TRSTAT == ENABLED);
        if(I2C2STATbits.BCL == 1) {
            result = 0;
        }
        else {
            I2CIdle();
        }
    }
    
    return result;
}

int I2CReadRegister(char regAddr, char* rxData, char slaveAddr) {
    int result = 1;
    char i = 0;
    char flag = 0;
    char j = 0;
    
    for(i = 0; i < 100; ++i) {
        I2CStart();
        I2CSendByte((slaveAddr << 1) | 0);  //request write to slave address
        I2CIdle();
        if(I2C2STATbits.ACKSTAT == 0) {
            flag = 1;
            break;
        }
    }
    if(flag != 1) {
        return 0;
    }
    flag = 0;
    I2CSendByte(regAddr);   //send address of register to read
    if(I2C2STATbits.ACKSTAT != 0) {
        return 0;
    }
    
    for(j = 0; j < 100; j++) {
        I2CStart(); //send repeat-start
        I2CSendByte((slaveAddr << 1) | 1);
        if(I2C2STATbits.ACKSTAT == 0) {
            flag = 1;
            break;
        }
    }
    if(flag != 1) {
        return 0;
    }
    flag = 0;
    
    //-----Below is where one byte is read from the slave device----
    
    I2CIdle();
    I2C2CONbits.RCEN = ENABLED; //Enable master read
    while(I2C2CONbits.RCEN == ENABLED);
    I2CIdle();
    I2C2STATbits.I2COV = DISABLED;
    *rxData = I2C2RCV;
    
    //now send NACK
    I2C2CONbits.ACKDT = 1;
    I2C2CONbits.ACKEN = 1;
    I2CStop();
    
    return result;
}

int I2CWriteBytes(char regAddr, char* txData, int len, char slaveAddr) {
    int result = 1;
    char flag = 0;
    char i = 0, j = 0;
    
    for(i = 0; i < 100; ++i) {
        I2CStart();
        I2CSendByte((slaveAddr << 1) | 0);
        I2CIdle();
        if(I2C2STATbits.ACKSTAT == 0) {
            flag = 1;
            break;
        }
    }
    if(flag == 0) {
        return 0;
    }
    flag = 0;
    I2CSendByte(regAddr);
    I2CIdle();
    
    for(j = 0; j < len; ++j) {
        if(I2C2STATbits.ACKSTAT == 0) {
            I2CSendByte(*(txData + j));
        }
        else {
            return 0;
        }
    }
    I2CStop();
    
    return result;
}