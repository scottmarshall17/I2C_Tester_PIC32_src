
#ifndef ADC_H
#define ADC_H

typedef enum bool_enum{
    false = 0, true = 1
} boolean;

void initADC(void);
char readSensors(void); //returns char with format 0b0000[Right][Top][Left][Middle]
int testSensor(char bitArray);
boolean frontTriggered(char sensors);
boolean rightTriggered(char sensors);
boolean leftTriggered(char sensors);
boolean middleTriggered(char sensors);

#endif