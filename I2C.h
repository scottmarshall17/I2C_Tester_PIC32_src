
#ifndef I2C_H
#define I2C_H

#include "LSM303CTypes.h"

void initI2C(void);
void I2CIdle(void);
int I2CStart(void);
int I2CStop(void);
int I2CWriteByte(char slaveAddr, char dataAddr, char data);
int I2CSendByte(char data);
int I2CReadRegister(char regAddr, char* rxData, char slaveAddr);
int I2CWriteBytes(char regAddr, char* txData, int len, char slaveAddr);
int MagSetConfigReg(char regAddr, char* txData);
int AccSetConfigReg(char regAddr, char* txData);
int MAG_SetODR(MAG_DO_t val);
int MAG_SetFullScale(MAG_FS_t val);
int MAG_BlockDataUpdate(MAG_BDU_t val);
int MAG_XY_AxOperativeMode(MAG_OMXY_t val);
int MAG_Z_AxOperativeMode(MAG_OMZ_t val);
int MAG_SetMode(MAG_MD_t val);
int MAG_TemperatureEN(MAG_TEMP_EN_t val);
int ACC_SetFullScale(ACC_FS_t val);
int ACC_BlockDataUpdate(ACC_BDU_t val);
int ACC_EnableAxis(char val);
int ACC_SetODR(ACC_ODR_t val);
int initACC(ACC_FS_t afs, ACC_BDU_t abu, char aea, ACC_ODR_t aodr);
int ACC_readZ(signed short* data);
int ACC_Status_Flags(char* val);
int MAG_readZ(signed short* data);
int MAG_readX(int* data);

#endif