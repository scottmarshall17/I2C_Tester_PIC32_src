
#include <xc.h>
#include "I2C.h"
#include "constants.h"
#include "LSM303CTypes.h"


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
int I2CRepeatStart() {
    int result = 1;
    I2CIdle();
    I2C2CONbits.RSEN = ENABLED;
    
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
    return I2CWriteBytes(dataAddr, &data, 1, slaveAddr);
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

int I2CReadBytes(char regAddr, char* rxData, int len, char slaveAddr) {
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
        //I2CStart(); //send repeat-start
        I2CRepeatStart();
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
    for(i = 0; i < len; ++i) {
        I2CIdle();
        I2C2CONbits.RCEN = ENABLED; //Enable master read
        while(I2C2CONbits.RCEN == ENABLED);
        I2CIdle();
        I2C2STATbits.I2COV = DISABLED;
        *(rxData+i) = I2C2RCV;

        if((i + 1) == len) {
            //now send NACK
            I2C2CONbits.ACKDT = 1;
            I2C2CONbits.ACKEN = 1;
            I2CStop();
        }
        else {
            I2C2CONbits.ACKDT = 0;  //Send ACK for sequential reads
            I2C2CONbits.ACKEN = 1;
        }
    
    }
    
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

int MagSetConfigReg(char regAddr, char* txData) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    
    if(I2CReadRegister(regAddr, &regData, magAddr) == 1) {
        
    }
    
    return result;
}
int MAG_SetODR(MAG_DO_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG1;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_DO_80_Hz;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_SetFullScale(MAG_FS_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG2;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_FS_16_Ga;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_BlockDataUpdate(MAG_BDU_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG5;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_BDU_ENABLE;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_XY_AxOperativeMode(MAG_OMXY_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG1;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_OMXY_ULTRA_HIGH_PERFORMANCE;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_Z_AxOperativeMode(MAG_OMZ_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG4;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_OMZ_ULTRA_HIGH_PERFORMANCE;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_SetMode(MAG_MD_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG3;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_MD_POWER_DOWN_2;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int MAG_TemperatureEN(MAG_TEMP_EN_t val) {
    int result = 1;
    char regData = 0;
    I2C_ADDR_t magAddr = MAG_I2C_ADDR;
    MAG_REG_t configReg = MAG_CTRL_REG1;
    
    if(I2CReadRegister(configReg, &regData, magAddr) == 1) {
        regData &= ~MAG_TEMP_EN_ENABLE;
        regData |= val;
    }
    else {
        return 0;
    }
    if(I2CWriteBytes(configReg, &regData, 1, magAddr) == 0) {
        return 0;
    }
    
    return result;
}

int AccSetConfigReg(char regAddr, char* txData) {
    int result = 1;
    
    return result;
}

int initMAG(MAG_DO_t modr, MAG_FS_t mfs,
    MAG_BDU_t mbu, MAG_OMXY_t mxyodr, MAG_OMZ_t mzodr, MAG_MD_t mm) {
    int result = 1;
    
    MAG_SetODR(modr);
    MAG_SetFullScale(mfs);
    MAG_BlockDataUpdate(mbu);
    MAG_XY_AxOperativeMode(mxyodr);
    MAG_Z_AxOperativeMode(mzodr);
    MAG_SetMode(mm);
    MAG_TemperatureEN(MAG_TEMP_EN_DISABLE);
    
    return result;
}

int MAG_readX(int* data) {
    int result = 1;
    char data_L = 0;
    char data_H = 0;
    MAG_XYZDA_t flag = MAG_XYZDA_NO;
    *data = 0;
    
    if(MAG_XYZ_AxDataAvailable(&flag)) {
        if(flag & MAG_XYZDA_YES) {
            if(I2CReadRegister(MAG_OUTX_L, &data_L, MAG_I2C_ADDR) != 1) {
                return 0;
            }
            if(I2CReadRegister(MAG_OUTX_H, &data_H, MAG_I2C_ADDR) != 1) {
                return 0;
            }
            *data = (int)((data_H << 8) | data_L);
        }
        else {
            return 0;
        }
    }
    else {
        result = 0;;
    }
    
    return result;
}

int MAG_XYZ_AxDataAvailable(MAG_XYZDA_t* value) {
    int result = 1;
    char data = 0;
    if(I2CReadRegister(MAG_STATUS_REG, &data, MAG_I2C_ADDR ) == 1) {
        *value = (MAG_XYZDA_t)((char)(data) & (char)MAG_XYZDA_YES);
    }
    else {
        *value = MAG_XYZDA_NO;
        return 0;
    }
    return result;
}

int MAG_readZ(signed short* data) {
    int result = 1;
    char data_L = 0;
    char data_H = 0;
    MAG_XYZDA_t flag = MAG_XYZDA_NO;
    *data = 0;
    
    if(MAG_XYZ_AxDataAvailable(&flag)) {
        if((flag & MAG_XYZDA_YES) == MAG_XYZDA_YES) {
            if(I2CReadRegister(MAG_OUTZ_L, &data_L, MAG_I2C_ADDR) != 1) {
                return 0;
            }
            if(I2CReadRegister(MAG_OUTZ_H, &data_H, MAG_I2C_ADDR) != 1) {
                return 0;
            }
            *data = ((data_H << 8) | data_L);
        }
        else {
            return 0;
        }
    }
    else {
        result = 0;;
    }
    
    return result;
}

int ACC_SetFullScale(ACC_FS_t val) {
    int result = 1;
    ACC_REG_t configReg = ACC_CTRL4;
    char data = 0;
    if(I2CReadRegister(configReg, &data, ACC_I2C_ADDR) == 1) {
        data &= ~ACC_FS_8g;
        data |= val;
        if(I2CWriteBytes(configReg, &data, 1, ACC_I2C_ADDR) == 1) {
            result = 1;
        }
        else {
            return 0;
        }
    }
    
    return result;
}
int ACC_BlockDataUpdate(ACC_BDU_t val) {
    int result = 1;
    ACC_REG_t configReg = ACC_CTRL1;
    char data = 0;
    if(I2CReadRegister(configReg, &data, ACC_I2C_ADDR) == 1) {
        data &= ~ACC_BDU_ENABLE;
        data |= val;
        if(I2CWriteBytes(configReg, &data, 1, ACC_I2C_ADDR) == 1) {
            result = 1;
        }
        else {
            return 0;
        }
    }
    
    return result;
}
int ACC_EnableAxis(char val) {
    int result = 1;
    ACC_REG_t configReg = ACC_CTRL1;
    char data = 0;
    if(I2CReadRegister(configReg, &data, ACC_I2C_ADDR) == 1) {
        data &= ~0x07;
        data |= val;
        if(I2CWriteBytes(configReg, &data, 1, ACC_I2C_ADDR) == 1) {
            result = 1;
        }
        else {
            return 0;
        }
    }
    
    return result;
}
int ACC_SetODR(ACC_ODR_t val) {
    int result = 1;
    ACC_REG_t configReg = ACC_CTRL1;
    char data = 0;
    if(I2CReadRegister(configReg, &data, ACC_I2C_ADDR) == 1) {
        data &= ~ACC_ODR_MASK;
        data |= val;
        if(I2CWriteBytes(configReg, &data, 1, ACC_I2C_ADDR) == 1) {
            result = 1;
        }
        else {
            return 0;
        }
    }
    
    return result;
}

int initACC(ACC_FS_t afs, ACC_BDU_t abu, char aea, ACC_ODR_t aodr) {
    int result = 1;
    ACC_SetFullScale(afs);
    ACC_BlockDataUpdate(abu);
    ACC_EnableAxis(aea);
    ACC_SetODR(aodr);
    return result;
}

int startBreakout(void) {
    int result = 1;
    initMAG(///// Magnetometer output data rate options
                  //MAG_DO_0_625_Hz,
                  //MAG_DO_1_25_Hz,
                  //MAG_DO_2_5_Hz,
                  //MAG_DO_5_Hz,
                  //MAG_DO_10_Hz,
                  //MAG_DO_20_Hz,
                  MAG_DO_40_Hz,
                  //MAG_DO_80_Hz,

                ///// Magnetic field full scale options
                  //MAG_FS_4_Ga,
                  //MAG_FS_8_Ga,
                  //MAG_FS_12_Ga,
                  MAG_FS_16_Ga,
                  
                ///// Magnetometer block data updating options
                  //MAG_BDU_DISABLE,
                  MAG_BDU_ENABLE,

                ///// Magnetometer X/Y axes output data rate
                  //MAG_OMXY_LOW_POWER,
                  //MAG_OMXY_MEDIUM_PERFORMANCE,
                  //MAG_OMXY_HIGH_PERFORMANCE,
                  MAG_OMXY_ULTRA_HIGH_PERFORMANCE,

                ///// Magnetometer Z axis output data rate
                  //MAG_OMZ_LOW_PW,
                  //MAG_OMZ_MEDIUM_PERFORMANCE,
                  //MAG_OMZ_HIGH_PERFORMANCE,
                  MAG_OMZ_ULTRA_HIGH_PERFORMANCE,

                ///// Magnetometer run mode
                  MAG_MD_CONTINUOUS
                  //MAG_MD_SINGLE
                  //MAG_MD_POWER_DOWN_1
                  //MAG_MD_POWER_DOWN_2
            );
    
    initACC(///// Acceleration full scale
                  ACC_FS_2g,
                  //ACC_FS_4g,
                  //ACC_FS_8g,

                ///// Accelerometer block data updating
                  //ACC_BDU_DISABLE,
                  ACC_BDU_ENABLE,

                ///// Enable X, Y, and/or Z axis
                  //ACC_DISABLE_ALL,
                  //ACC_X_ENABLE,
                  //ACC_Y_ENABLE,
                  //ACC_Z_ENABLE,
                  //ACC_X_ENABLE|ACC_Y_ENABLE,
                  //ACC_X_ENABLE|ACC_Z_ENABLE,
                  //ACC_Y_ENABLE|ACC_Z_ENABLE,
                  ACC_X_ENABLE|ACC_Y_ENABLE|ACC_Z_ENABLE,

                ///// Accelerometer output data rate
                  //ACC_ODR_POWER_DOWN
                  //ACC_ODR_10_Hz
                  ACC_ODR_50_Hz
                  //ACC_ODR_100_Hz
                  //ACC_ODR_200_Hz
                  //ACC_ODR_400_Hz
                  //ACC_ODR_800_Hz
            );
    return result;
}

int ACC_Status_Flags(char* val) {
    int result = 1;
    if(I2CReadRegister(ACC_STATUS, val, ACC_I2C_ADDR) == 1) {
        result = 1;
    }
    else {
        result = 0;
    }
    return result;
}

int ACC_readZ(signed short* data) {
    int result = 1;
    char data_L = 0;
    char data_H = 0;
    char flag_ACC_STATUS_FLAGS = ACC_Z_NEW_DATA_AVAILABLE;
    
    if(ACC_Status_Flags(&flag_ACC_STATUS_FLAGS)==1) {
        result = 1;
    }
    else {
        return 0;
    }
    if(flag_ACC_STATUS_FLAGS & ACC_Z_NEW_DATA_AVAILABLE) {
        if(I2CReadRegister(ACC_OUT_Z_L, &data_L, ACC_I2C_ADDR) != 1) {
            return 0;
        }
        if(I2CReadRegister(ACC_OUT_Z_H, &data_H, ACC_I2C_ADDR) != 1) {
            return 0;
        }
        *data = ((data_H << 8) | data_L);
    }
    
    return result;
}