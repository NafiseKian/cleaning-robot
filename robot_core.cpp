/**
** robot_core.cpp
** author : Nafise Kian 
** date : 01/04/2024
** this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include <thread> 
#include <mutex> 
#include <condition_variable> 
#include <chrono> 
#include <cmath>
#include <unistd.h> 
#include <string>
#include "gps_module.h"
#include "camera_module.h"
#include "drive_module.h"
#include "ultra_sensor.h"
#include "wifi_module.h"
#include "network_module.h"

// Global variables for synchronization
std::mutex mtx;
std::condition_variable cv;
bool stopMovement = false;
bool photoTaken = false;
bool trashDetected = false;

void gps_wifi_thread() {
    NetworkModule network("34.165.89.174", 3389);

    GPSModule gps;
    int k = 3;

    Localization wifi;
    std::vector<std::tuple<std::string, double, double, double>> fingerprintData = wifi.readWiFiFingerprintFile("wifi_fingerprint.txt");

    while (true) {
        std::cout << "worker thread loop" << std::endl;

        std::string gpsData;
        if (gps.readData(gpsData)) {
            // Process the received GPS data in gpsData
            std::cout << "Received GPS data: " << gpsData << std::endl;
            // ... (parse and extract relevant information)
        } else {
            // Handle the case where no data is available
            std::cout << "No GPS data available yet." << std::endl;
        }

        std::string ret = wifi.captureWifiSignal();
        std::cout << "------------------------wifi signal levels -----------------------" << std::endl;
        std::cout << ret << std::endl;
        std::vector<std::pair<std::string, double>> observedRSSI = wifi.parseIwlistOutput(ret);

        std::tuple<double, double> estimatedLocation = wifi.knnLocation(fingerprintData, observedRSSI, k);
        std::cout << "Estimated location using KNN: X = " << std::get<0>(estimatedLocation)
                  << ", Y = " << std::get<1>(estimatedLocation) << std::endl;


        if (network.connectToServer()) {
            std::cout << "Connected to server successfully." << std::endl;
            network.sendData("ROBOT," + std::to_string(std::get<0>(estimatedLocation)) + "," + std::to_string(std::get<1>(estimatedLocation)));
        } else {
            std::cout << "Failed to connect to server." << std::endl;
        }

        //TODO : if out of that boundary notify movement thread to turn

        sleep(5);
    }
}

void camera_thread(int &photoCounter) 
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return stopMovement; });

        CameraModule::capturePhoto(photoCounter);
        std::cout << "Photo " << photoCounter << " taken." << std::endl;

        //TODO : pass image to pythin side to process it 
        std::cout << "Processing image " << photoCounter << "..." << std::endl;

        /*TODO :get the response from ai and set the condition
        if (trash == 0) { 
            trashDetected = true;
            std::cout << "Trash detected in photo " << photoCounter << "!" << std::endl;
        } else {
            trashDetected = false;
            std::cout << "No trash detected in photo " << photoCounter << "." << std::endl;
        }
        */

        photoCounter++;

        // Signal main thread to resume movement
        photoTaken = true;
        stopMovement = false;
        cv.notify_all();

        
    }
}

int main() {
    std::thread gpsWifiThread(gps_wifi_thread);

    // Initialize motor control
    MotorControl::setup();
    std::cout << "Motor controller set up done" << std::endl;

    int photoCounter = 0;
    std::thread camThread(camera_thread, std::ref(photoCounter));

    UltrasonicSensor frontSensorL("Front-left", 26, 24);
    UltrasonicSensor frontSensorR("Front-right", 20, 21);
    UltrasonicSensor rightSensor("Right", 22, 27);
    UltrasonicSensor leftSensor("Left", 18, 17);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !stopMovement; });

        int distanceFrontL = frontSensorL.getDistanceCm();
        int distanceFrontR = frontSensorR.getDistanceCm();
        int distanceRight = rightSensor.getDistanceCm();
        int distanceLeft = leftSensor.getDistanceCm();

        // Output the distances to help with debugging
        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;

        // Handle -1 sensor errors
        bool validFrontL = (distanceFrontL != -1 && distanceFrontL < 20);
        bool validFrontR = (distanceFrontR != -1 && distanceFrontR < 20);
        bool validRight = (distanceRight != -1 && distanceRight < 20);
        bool validLeft = (distanceLeft != -1 && distanceLeft < 20);

        if (validFrontL || validFrontR) {
            // Stop the robot and notify the camera thread
            MotorControl::stop();
            std::cout << "Obstacle detected. Stopping and taking a photo..." << std::endl;
            stopMovement = true;
            photoTaken = false;
            cv.notify_all();

            // Wait for the camera thread to finish taking the photo and processing it
            cv.wait(lock, [] { return photoTaken; });

            if (trashDetected) {
                std::cout << "Trash detected. Moving closer to pick it up..." << std::endl;
                // TODO : Add code to move closer to the trash and pick it up
            }

            // Continue moving after handling the detected trash
            std::cout << "Resuming movement..." << std::endl;
        }
        else if(validRight)
        {
            MotorControl::turnLeft();
            usleep(500000);
            MotorControl::forward();
        }
        else if(validLeft)
        {
            MotorControl::turnRight();
            usleep(500000);
            MotorControl::forward();
        }
        else if ((validFrontL || validFrontR) && validLeft && validRight )
        {
            MotorControl::backward();
            usleep(500000);
            MotorControl::turnRight();
            usleep(500000);
        }
        else {
            // If no valid obstacle is directly in front, move forward
            MotorControl::forward();
            std::cout << "Path is clear. Moving forward..." << std::endl;
        }

        usleep(500000); // 0.5 second delay for general loop control
    }

    return 0;
}
