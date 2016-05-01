
#ifndef I2C_H
#define I2C_H

void initI2C(void);
void I2CIdle(void);
int I2CStart(void);
int I2CStop(void);
int I2CWriteByte(char slaveAddr, char dataAddr, char data);
int I2CSendByte(char data);
int I2CReadRegister(char regAddr, char* rxData, char slaveAddr);
int I2CWriteBytes(char regAddr, char* txData, int len, char slaveAddr);


#endif