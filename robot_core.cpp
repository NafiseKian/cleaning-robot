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
#include "wifi_module.h"

int main() 
{
    // Initialize motor control
    MotorControl::setup();
    std::cout <<"motor controller set up done"<< std::endl;

    int photoCounter = 0;

    GPSModule gps;

    std::vector<Localization::AccessPoint> access_points = {
      {1.0, 1.0, "0A:96:71:47:4C:FF"}, // JN extension
      {5.0, 1.0, "F6:43:35:08:ED:BD"},  // JN
      {3.0, 5.0, "2A:F4:8D:C2:36:E5"}   // recordreaker
    };

    Localization wifi_loc(access_points);

    // Get current position based on measured distances from WiFi packets
    std::pair<double, double> estimated_position = wifi_loc.getCurrentPositionFromWiFi();

    if (!std::isnan(estimated_position.first)) {
        std::cout << "Estimated Robot Position (x, y): " << estimated_position.first << ", " << estimated_position.second << std::endl;
    } else {
        std::cerr << "Error: Robot location estimation failed." << std::endl;
    }
    


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

        
        // Move forward for 2 seconds
        MotorControl::forward();
        std::cout <<"called forward"<< std::endl;
        sleep(2); // Sleep for 2 seconds
        MotorControl::stop(); // Stop moving

        // Capture a photo
        CameraModule::capturePhoto(photoCounter);
        
        //call object detection
        //call arm module to pick the trash 

        if(photoCounter==2) break ; 
    }

    return 0;
}