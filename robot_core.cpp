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
        auto fingerprintData = wifi.readWiFiFingerprintFile("wifi_fingerprint.txt");
        std::pair<double, double> location = wifi.findLocation(fingerprintData, observedRSSI);
        std::cout<<"best location is --->"<<location.first<<"      "<<location.second << std::endl;
        
        sleep(5);
        
    }
    

}

const int OBSTACLE_CLOSE = 20;
const int OBSTACLE_NEAR = 30;
const int ERROR_VALUE = 999;
const int SLEEP_AFTER_TURN = 2;
const int SLEEP_FORWARD = 3;

int getValidDistance(Sensor sensor) {
    int distance = sensor.getDistanceCm();
    return (distance == -1) ? ERROR_VALUE : distance;
}

void handleMovement(int frontLeft, int frontRight, int right, int left) {
    if ((frontLeft < OBSTACLE_CLOSE || frontRight < OBSTACLE_CLOSE) && left > OBSTACLE_CLOSE && right > OBSTACLE_CLOSE) {
        if (frontLeft < frontRight) {
            MotorControl::turnRight();
            std::cout << "Obstacle detected in front. Turning right." << std::endl;
        } else {
            MotorControl::turnLeft();
            std::cout << "Obstacle detected in front. Turning left." << std::endl;
        }
        sleep(SLEEP_AFTER_TURN);
    } else if (right < OBSTACLE_CLOSE && left > OBSTACLE_CLOSE && frontRight > OBSTACLE_NEAR) {
        MotorControl::turnLeft();
        std::cout << "Obstacle detected on the right. Turning left." << std::endl;
        sleep(SLEEP_AFTER_TURN);
    } else if (left < OBSTACLE_CLOSE && right > OBSTACLE_CLOSE && frontLeft > OBSTACLE_NEAR) {
        MotorControl::turnRight();
        std::cout << "Obstacle detected on the left. Turning right." << std::endl;
        sleep(SLEEP_AFTER_TURN);
    } else if (frontLeft < OBSTACLE_NEAR && frontRight < OBSTACLE_NEAR) {
        MotorControl::turnRight();
        std::cout << "Obstacles too close in front. Executing turn." << std::endl;
        sleep(SLEEP_AFTER_TURN);
    } else {
        MotorControl::forward();
        std::cout << "Path is clear. Moving forward." << std::endl;
        sleep(SLEEP_FORWARD);
    }
}

int main() 
{

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
    int distanceFrontL = getValidDistance(frontSensorL);
    int distanceFrontR = getValidDistance(frontSensorR);
    int distanceRight = getValidDistance(rightSensor);
    int distanceLeft = getValidDistance(leftSensor);

    handleMovement(distanceFrontL, distanceFrontR, distanceRight, distanceLeft);

        MotorControl::forward();
        sleep(2);
        MotorControl::turnLeft();
        sleep(1);
        //MotorControl::stop();
        //sleep(5);
        //MotorControl::forward();
        //sleep(5);
        MotorControl::forward();
        sleep(3);
        MotorControl::turnRight();
        sleep(1);
        MotorControl::forward();
        sleep(3);
        MotorControl::stop(); // Stop moving
        // Capture a photo
        CameraModule::capturePhoto(photoCounter);
        sleep(5);
        
        //call object detection
        //call arm module to pick the trash 

        if(photoCounter==3) break ; 
    }
    



        
        
       
    

    

    return 0;
}