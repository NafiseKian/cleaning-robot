
#include "drive_module.h"
#include <pigpio.h>
#include <unistd.h>
#include <iostream>

#define enA 5  
#define in1 7  
#define in2 8  
#define in3 9  
#define in4 10 
#define enB 6  

namespace MotorControl {

void setup() {
    if (gpioInitialise() < 0) {
          printf("set up failed \n");
        return;
    }

    gpioSetMode(enA, PI_OUTPUT);
    printf("enA setupped \n");
    gpioSetMode(in1, PI_OUTPUT);
    gpioSetMode(in2, PI_OUTPUT);
    gpioSetMode(in3, PI_OUTPUT);
    gpioSetMode(in4, PI_OUTPUT);
    gpioSetMode(enB, PI_OUTPUT);
    printf("enB setupped \n");

    
   
}

void forward(int speed) 
{
    if (gpioPWM(enA, speed) != 0 || gpioPWM(enB, speed) != 0) {
        printf( "Failed to set PWM on Enable pins." );
        return;
    }

    if (gpioWrite(in1, PI_HIGH) != 0 ||
        gpioWrite(in2, PI_LOW) != 0 ||
        gpioWrite(in3, PI_LOW) != 0 ||
        gpioWrite(in4, PI_HIGH) != 0) {
        printf( "Failed to set motor direction pins.");
        return;
    }

    printf("Motors should be moving forward now." );
}

void backward(int speed) 
{
    if (gpioPWM(enA, speed) != 0 || gpioPWM(enB, speed) != 0) {
        printf( "Failed to set PWM on Enable pins." );
        return;
    }

    if (gpioWrite(in1, PI_LOW) != 0 ||
        gpioWrite(in2, PI_HIGH) != 0 ||
        gpioWrite(in3, PI_HIGH) != 0 ||
        gpioWrite(in4, PI_LOW) != 0) {
        printf( "Failed to set motor direction pins.");
        return;
    }

    printf("Motors should be moving forward now." );
}

void turnRight(int speed)
{
    if (gpioPWM(enA, speed) != 0 || gpioPWM(enB, speed) != 0) {
        printf( "Failed to set PWM on Enable pins." );
        return;
    }
   
    if (gpioWrite(in1, PI_LOW) != 0 ||
        gpioWrite(in2, PI_HIGH) != 0 ||
        gpioWrite(in3, PI_LOW) != 0 ||
        gpioWrite(in4, PI_HIGH) != 0) {
        printf( "Failed to set motor direction pins.");
        return;
    }

    printf("Motors should be turning to right now." );
}


void turnLeft(int speed)
{
    if (gpioPWM(enA, speed) != 0 || gpioPWM(enB, speed) != 0) {
        printf( "Failed to set PWM on Enable pins." );
        return;
    }
   

    if (gpioWrite(in1, PI_HIGH) != 0 ||
        gpioWrite(in2, PI_LOW) != 0 ||
        gpioWrite(in3, PI_HIGH) != 0 ||
        gpioWrite(in4, PI_LOW) != 0) {
        printf( "Failed to set motor direction pins.");
        return;
    }

    printf("Motors should be turning to left now." );
}

void stop() {
    gpioWrite(in1, PI_LOW);
    gpioWrite(in2, PI_LOW);
    gpioWrite(in3, PI_LOW);
    gpioWrite(in4, PI_LOW);
}

}


