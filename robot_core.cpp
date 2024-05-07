/**
** robot_core.cpp
** author : Nafise Kian 
** date : 01/04/2024
** this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include <thread> // For std::thread
#include <mutex>  // For std::mutex
#include <condition_variable> // For std::condition_variable
#include <chrono> // For std::chrono for timing
#include<math.h>
//#include <Python.h>
#include <unistd.h> // For sleep()

#include "gps_module.h"
#include "camera_module.h"
#include "drive_module.h"
#include "ultra_sensor.h"
#include "wifi_module.h"


void* gps_wifi_thread(void* args)
{
    GPSModule gps;

    std::vector<AccessPoint> access_points = {
      {1.0, 1.0, "0A:96:71:47:4C:FF"}, // JN extension
      {5.0, 1.0, "F6:43:35:08:ED:BD"},  // JN
      {3.0, 5.0, "2A:F4:8D:C2:36:E5"}   // recordreaker
    };

    Localization wifi(access_points);

    while (true)
    {
        std::cout<<"worker thread loop"<<std::endl ;


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

         
        std::string ret = wifi.captureWifiSignal();
        std::cout<<"------------------------wifi signal levels -----------------------"<<std::endl;
        std::cout<<ret<<std::endl;
        std::vector<std::pair<std::string, double>> observedRSSI = wifi.parseIwlistOutput(ret);
        std::vector<std::tuple<std::string, double, double, double>> fingerprintData = wifi.readWiFiFingerprintFile("wifi_fingerprint.txt");
        std::tuple<double, double> location = wifi.findLocation(fingerprintData, observedRSSI);
        std::cout << "best location is ---> " << std::get<0>(location) << "      " << std::get<1>(location) << std::endl;

        sleep(5);
        
    }
    

}


int main() {

    pthread_t cThread;
    if(pthread_create(&cThread, NULL, &gps_wifi_thread,NULL)){
        perror("ERROR creating thread.");
    }


    // Initialize motor control
    MotorControl::setup();
    std::cout <<"motor controller set up done"<< std::endl;

    int photoCounter = 0;

    

    /*
    // Get current position based on measured distances from WiFi packets
    std::pair<double, double> estimated_position = wifi_loc.getCurrentPositionFromWiFi();

    if (!isnan(estimated_position.first)) {
        std::cout << "Estimated Robot Position (x, y): " << estimated_position.first << ", " << estimated_position.second << std::endl;
    } else {
        std::cerr << "Error: Robot location estimation failed." << std::endl;
    }
    */

    UltrasonicSensor frontSensorL("Front-left", 26, 24);
    UltrasonicSensor frontSensorR("Front-right", 20, 21);
    UltrasonicSensor rightSensor("Right", 22, 27);
    UltrasonicSensor leftSensor("Left", 18 , 17);


 while (true) {
        int distanceFrontL = frontSensorL.getDistanceCm();
        int distanceFrontR = frontSensorR.getDistanceCm();
        int distanceLeft = leftSensor.getDistanceCm();
        int distanceRight = rightSensor.getDistanceCm();

        // Output the distances to help with debugging
        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;
        std::cout << "Left Distance: " << distanceLeft << " cm" << std::endl;
        std::cout << "Right Distance: " << distanceRight << " cm" << std::endl;

      // Handle -1 sensor errors
        bool validFrontL = (distanceFrontL != -1 && distanceFrontL < 20);
        bool validFrontR = (distanceFrontR != -1 && distanceFrontR < 20);
        bool validLeft = (distanceLeft != -1 && distanceLeft > 20);
        bool validRight = (distanceRight != -1 && distanceRight > 20);

         if (validFrontL || validFrontR) {
            if (validLeft && validRight) {
                if (validFrontL && (!validFrontR || distanceFrontL < distanceFrontR)) {
                    MotorControl::turnRight();
                    std::cout << "Obstacle closer on left; turning right." << std::endl;
                } else {
                    MotorControl::turnLeft();
                    std::cout << "Obstacle closer on right; turning left." << std::endl;
                }
            } else if (validLeft) {
                MotorControl::turnLeft();
                std::cout << "Right side blocked or invalid; turning left." << std::endl;
            } else if (validRight) {
                MotorControl::turnRight();
                std::cout << "Left side blocked or invalid; turning right." << std::endl;
            } else {
                // If both sides and front are blocked, reverse
                MotorControl::backward();
                std::cout << "Blocked on all sides; moving backward." << std::endl;
            }
            usleep(1000000); // Wait 1 second to give time for the maneuver to be executed
        } else {
            // If no valid obstacle is directly in front, move forward
            MotorControl::forward();
            std::cout << "Path is clear or sensor data invalid. Moving forward." << std::endl;
        }

        usleep(500000); // 0.5 second delay for general loop control
    }

    return 0;
}