#include <xc.h>
#include "constants.h"
#include "control.h"

void turnRight(void){
    //If we want the robot to turn right, we will stop the left tire, 
    //and have the right tire slow down until it has turned a sufficient amount
    OC3RS = 600;
    OC4RS = 0;
}

void turnLeft(void){
    //If we want the robot to turn left, we will stop the right tire, 
    //and have the left tire slow down until it has turned a sufficient amount
    OC3RS = 0;
    OC4RS = 600;
}

void goForward(void){
    //If we want the robot to go straight, we will have both tires 
    //go forward at a middle speed
    OC3RS = 768;
    OC4RS = 768;
}