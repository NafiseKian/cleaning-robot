#include <Python.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <libserialport.h>
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

const double X_MIN = 0.0;
const double X_MAX = 100.0;
const double Y_MIN = 0.0;
const double Y_MAX = 100.0;

#define SOCKET_PATH "/tmp/unix_socket_example"

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
            std::cout << "Received GPS data: " << gpsData << std::endl;
        } else {
            std::cout << "No GPS data available yet." << std::endl;
        }

        std::string ret = wifi.captureWifiSignal();
        std::cout << "------------------------wifi signal levels -----------------------" << std::endl;
        std::cout << ret << std::endl;
        std::vector<std::pair<std::string, double>> observedRSSI = wifi.parseIwlistOutput(ret);

        std::tuple<double, double> estimatedLocation = wifi.knnLocation(fingerprintData, observedRSSI, k);
        double currentX = std::get<0>(estimatedLocation);
        double currentY = std::get<1>(estimatedLocation);
        std::cout << "Estimated location using KNN: X = " << currentX
                  << ", Y = " << currentY << std::endl;

        if (network.connectToServer()) {
            std::cout << "Connected to server successfully." << std::endl;
            network.sendData("ROBOT," + std::to_string(currentX) + "," + std::to_string(currentY));
        } else {
            std::cout << "Failed to connect to server." << std::endl;
        }

        sleep(5);
    }
}

void camera_thread(int &photoCounter) {
    int sockfd;
    struct sockaddr_un addr;
    
    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // Set socket address
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (true) {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return stopMovement; });

        std::string path = CameraModule::capturePhoto(photoCounter);
        std::cout << "Photo " << photoCounter << " taken." << std::endl;

        // Send the image path to Python process
        if (send(sockfd, path.c_str(), path.length(), 0) == -1) {
            perror("send error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Receive the detection result from Python process
        char buffer[256];
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n == -1) {
            perror("recv error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
        trashDetected = std::string(buffer) == "1";

        // Print the detection result
        if (trashDetected) {
            std::cout << "Trash detected in photo " << photoCounter << "!" << std::endl;
        } else {
            std::cout << "No trash detected in photo " << photoCounter << "." << std::endl;
        }

        photoCounter++;

        // Signal main thread to resume movement
        photoTaken = true;
        stopMovement = false;
        cv.notify_all();

        if (photoCounter == 10) break;
    }

    // Close socket
    close(sockfd);
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

        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;
        std::cout << "Right Distance : " << distanceRight << " cm" << std::endl;
        std::cout << "Left Distance : " << distanceLeft << " cm" << std::endl;

        bool validFrontL = (distanceFrontL != -1 && distanceFrontL < 20);
        bool validFrontR = (distanceFrontR != -1 && distanceFrontR < 20);
        bool validRight = (distanceRight != -1 && distanceRight < 20);
        bool validLeft = (distanceLeft != -1 && distanceLeft < 20);

        if (validFrontL || validFrontR) {
            MotorControl::stop();
            std::cout << "Obstacle detected. Stopping and taking a photo..." << std::endl;
            usleep(500000);
            MotorControl::backward();
            usleep(500000);
            MotorControl::turnRight();
            usleep(500000);

            stopMovement = true;
            photoTaken = false;
            cv.notify_all();

            cv.wait(lock, [] { return photoTaken; });

            if (trashDetected) {
                std::cout << "Trash detected. Moving closer to pick it up..." << std::endl;
                MotorControl::forward();
                usleep(500000); // Move forward for half second to get closer to the trash
                MotorControl::stop();
                std::cout << "Picking up trash..." << std::endl;
                sleep(1); // Simulate pick-up delay
                //sendCommandToArduino(serialPort, 'P'); // Send pick command to Arduino
            }

            std::cout << "Resuming movement..." << std::endl;
        } else if (validRight) {
            MotorControl::turnLeft();
            usleep(500000);
            MotorControl::forward();
        } else if (validLeft) {
            MotorControl::turnRight();
            usleep(500000);
            MotorControl::forward();
        } else if ((validFrontL || validFrontR) && validLeft && validRight) {
            MotorControl::backward();
            usleep(500000);
            MotorControl::turnRight();
            usleep(500000);
        } else {
            // If no valid obstacle is directly in front, move forward
            MotorControl::forward();
            std::cout << "Path is clear. Moving forward..." << std::endl;
        }

        usleep(500000); // 0.5 second delay for general loop control
    }

    return 0;
}
