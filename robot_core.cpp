/**
**this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include <Python.h>
#include <unistd.h> // For sleep()
#include "camera_module.h"
#include "drive_module.h"

#include <pigpio.h>
#include <stdio.h>


#define enA 5  
#define in1 7  
#define in2 8  
#define in3 9  
#define in4 10 
#define enB 6  

void setup() {
    if (gpioInitialise() < 0) {
        printf("GPIO initialization failed\n");
        return;
    }

    gpioSetMode(enA, PI_OUTPUT);
    gpioSetMode(in1, PI_OUTPUT);
    gpioSetMode(in2, PI_OUTPUT);
    gpioSetMode(in3, PI_OUTPUT);
    gpioSetMode(in4, PI_OUTPUT);
    gpioSetMode(enB, PI_OUTPUT);

    printf("GPIO setup done\n");
}

void forward() {
    gpioPWM(enA, 180);
    gpioPWM(enB, 180);

    gpioWrite(in1, PI_LOW);
    gpioWrite(in2, PI_HIGH);
    gpioWrite(in3, PI_HIGH);
    gpioWrite(in4, PI_LOW);

    printf("Motors moving forward\n");
}

void stop() {
    gpioWrite(in1, PI_LOW);
    gpioWrite(in2, PI_LOW);
    gpioWrite(in3, PI_LOW);
    gpioWrite(in4, PI_LOW);

    printf("Motors stopped\n");
}



int main() 
{
    int photoCounter = 0;

// Initialize GPIO
    if (gpioInitialise() < 0) {
        printf("GPIO initialization failed\n");
        return 1;
    }

    // Setup motor control
    setup();
     printf("set up done\n\n\n");
    // Move forward for a few seconds
    forward();
    printf("called forward\n\n\n");
    sleep(5); // Move forward for 5 seconds
    
    stop();

    // Cleanup GPIO
    gpioTerminate();

    // Initialize motor control
    //MotorControl::setup();
   
    // Move forward for 2 seconds
    //MotorControl::forward();
    
    //sleep(7); // Sleep for 2 seconds
    //MotorControl::stop(); // Stop moving

    // Capture a photo
    //CameraModule::capturePhoto(photoCounter);

    return 0;
}
