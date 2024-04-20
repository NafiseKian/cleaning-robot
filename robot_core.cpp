/**
**this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include <Python.h>
#include <unistd.h> // For sleep()
#include "camera_module.h"
#include "drive_module.h"

int main() 
{
    int photoCounter = 0;

    // Initialize motor control
    MotorControl::setup();
    printf("set up done\n\n\n");
    // Move forward for 2 seconds
    MotorControl::forward();
    printf("called forward\n\n\n");
    sleep(7); // Sleep for 2 seconds
    MotorControl::stop(); // Stop moving

    // Capture a photo
    CameraModule::capturePhoto(photoCounter);

    return 0;
}