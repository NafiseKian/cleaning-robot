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
        // Assuming sensor classes are correctly defined and returning the current distance in cm
        int distanceFrontL = frontSensorL.getDistanceCm();
        int distanceFrontR = frontSensorR.getDistanceCm();
        int distanceRight = rightSensor.getDistanceCm();
        int distanceLeft = leftSensor.getDistanceCm();

        // Debug outputs to trace sensor values
        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;
        std::cout << "Right Distance: " << distanceRight << " cm" << std::endl;
        std::cout << "Left Distance: " << distanceLeft << " cm" << std::endl;

              if (distanceFrontL < 20 || distanceFrontR < 20) {
            // Determine which direction to turn based on the shorter distance
            if (distanceFrontL < distanceFrontR) {
                MotorControl::turnRight();
                std::cout << "Obstacle detected in front. Turning right." << std::endl;
            } else {
                MotorControl::turnLeft();
                std::cout << "Obstacle detected in front. Turning left." << std::endl;
            }
            usleep(2000000); // Wait 2 seconds to give time for the turn to be executed
        } else {
            // If no obstacle is directly in front, move forward
            MotorControl::forward();
            std::cout << "Path is clear. Moving forward." << std::endl;
        }

        usleep(500000); // 0.5 second delay for general loop control
    }

    return 0;
}

//     while (true) 
//     {
//         int distanceFrontL = frontSensorL.getDistanceCm();
//         int distanceFrontR = frontSensorR.getDistanceCm();
//         int distanceRight = rightSensor.getDistanceCm();
//         int distanceLeft = leftSensor.getDistanceCm();

//         std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
//         std::cout << "Front Distance sensor right : " << distanceFrontR << " cm" << std::endl;
//         std::cout << "Right Distance: " << distanceRight << " cm" << std::endl;
//         std::cout << "Left Distance: " << distanceLeft << " cm" << std::endl;

//         if ((distanceFrontL < 20 || distanceFrontR < 20) && distanceLeft > 20 && distanceRight > 20) 
//         {
//             if (distanceFrontL < distanceFrontR)
//                 MotorControl::turnRight();
//             else
//                 MotorControl::turnLeft();
//             std::cout << "Obstacle detected in front. Turning." << std::endl;
//             sleep(2); // Sleep for 2 seconds after turning
//         } else if (distanceRight < 20 && distanceLeft > 20 && distanceFrontR > 20) {
//             MotorControl::turnLeft();
//             std::cout << "Obstacle detected on the right. Turning left." << std::endl;
//             sleep(2); // Sleep for 2 seconds after turning
//         } else if (distanceLeft < 20 && distanceRight > 20 && distanceFrontL > 20) {
//             MotorControl::turnRight();
//             std::cout << "Obstacle detected on the left. Turning right." << std::endl;
//             sleep(2); // Sleep for 2 seconds after turning
//         } else if (distanceFrontL < 20 && distanceFrontR < 20) {
//             MotorControl::stop(); // Or any suitable maneuver
//             std::cout << "Obstacles too close in front. Stopping." << std::endl;
//             sleep(2); // Sleep for 2 seconds after turning
//         }
//          else if (distanceFrontL < 5 && distanceFrontR < 5) {
//             MotorControl::backward(); // Or any suitable maneuver
//             std::cout << "Obstacles too close in front. Turning." << std::endl;
//             sleep(4); // Sleep for 2 seconds after turning
//         } 
//         else 
//         {
//             MotorControl::forward();
//             std::cout << "Path is clear. Moving forward." << std::endl;
//             CameraModule::capturePhoto(photoCounter);
//             sleep(3); // Continue moving forward for 3 seconds
//         }

//         sleep(5);
//         if(photoCounter==3) break ; 
//     }

    

//     return 0;
// }