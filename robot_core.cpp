/**
**this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include <Python.h>
#include <unistd.h> // For sleep()

#include "gps_module.h"
#include "camera_module.h"
#include "drive_module.h"
#include "ultra_sensor.h"

int main() 
{
    int photoCounter = 0;
    GPSModule gps;
    //UltrasonicSensor sensor1 = UltrasonicSensor();

    while(true)
    {
        std::string gpsData;
        if (gps.readData(gpsData)) {
            // Process the received GPS data in gpsData
            std::cout << "Received GPS data: " << gpsData << std::endl;
            // ... (parse and extract relevant information)
        }
        else 
        {
            // Handle the case where no data is available
            std::cout << "No GPS data available yet." << std::endl;
        }

        // Initialize motor control
        MotorControl::setup();
        std::cout <<"set up done"<< std::endl;
        // Move forward for 2 seconds
        MotorControl::forward();
        std::cout <<"called forward"<< std::endl;
        sleep(2); // Sleep for 2 seconds
        MotorControl::stop(); // Stop moving

        // Capture a photo
        CameraModule::capturePhoto(photoCounter);

        if(photoCounter==3) break ; 
    }

    return 0;
}