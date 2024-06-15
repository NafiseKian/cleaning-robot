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
#include <csignal>
#include <libserialport.h>
#include <string>
#include <sstream>
#include <atomic>

#include "gps_module.h"
#include "camera_module.h"
#include "drive_module.h"
#include "ultra_sensor.h"
#include "wifi_module.h"
#include "network_module.h"
#include "arm_module.h"

// Global variables for synchronization
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> stopMovement(false);
std::atomic<bool> photoTaken(false);
std::atomic<bool> trashDetected(false);
std::atomic<bool> stopProgram(false);
std::atomic<bool> userStopMovement(false);
std::atomic<bool> navigateToCharger(false);
std::atomic<double> currentX(0.0);
std::atomic<double> currentY(0.0);
std::string trashLocation = "center";

// Coordinates for the charging station
const double CHARGER_X = 10.0;
const double CHARGER_Y = 10.0;

#define SOCKET_PATH "/tmp/unix_socket_example"

// Signal handler to stop the motors and exit the program
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // Stop the motors
    MotorControl::stop();
    // Signal the program to stop
    stopProgram.store(true);
    // Notify all threads to exit
    cv.notify_all();
}

void gps_wifi_thread() {
    NetworkModule network("34.165.89.174", 3389);

    GPSModule gps;
    int k = 3;

    Localization wifi;
    std::vector<std::tuple<std::string, double, double, double>> fingerprintData = wifi.readWiFiFingerprintFile("wifi_fingerprint.txt");

    while (!stopProgram.load()) {
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

    while (!stopProgram.load()) {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return stopMovement.load() || stopProgram.load(); });

        if (stopProgram.load()) break;

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
        std::cout << "Received from Python: " << buffer << std::endl;  // Debug print
        
        // Parse the buffer
        std::string trashDetectedFlag, placeOfTrash, angle;
        std::istringstream ss(buffer);
        std::getline(ss, trashDetectedFlag, '|');
        std::getline(ss, placeOfTrash, '|');
        std::getline(ss, angle, '|');

        trashDetected.store(trashDetectedFlag == "1");

        // Print the detection result
        if (trashDetected.load()) {
            std::cout << "Trash detected in photo " << photoCounter << "!" << std::endl;
            trashLocation = placeOfTrash ; 
        } else {
            std::cout << "No trash detected in photo " << photoCounter << "." << std::endl;
        }

        photoCounter++;

        // Signal main thread to resume movement
        photoTaken.store(true);
        stopMovement.store(false);
        cv.notify_all();
    }

    // Close socket
    close(sockfd);
}

void user_input_thread() {
    std::string input;
    while (!stopProgram.load()) {
        std::cin >> input;
        if (input == "s") {
            userStopMovement.store(true);
        } else if (input == "c") {
            userStopMovement.store(false);
            cv.notify_all();
        } 
        else if (input == "ch") 
        {
            navigateToCharger.store(true);
            userStopMovement.store(false);
            cv.notify_all();
        }
        else if (input =="t")
        {
            MotorControl::stop();
            // Signal the program to stop
            stopProgram.store(true);
            // Notify all threads to exit
            cv.notify_all();
        }
    }
}

// Function to navigate the robot towards the charging station
void navigate_to_charger() {
    const double TOLERANCE = 1.0;  // Acceptable distance to the charging station

    while (true) {
        // Get the current estimated position
        double currentPosX = currentX.load();
        double currentPosY = currentY.load();

        if (std::abs(currentPosX - CHARGER_X) <= TOLERANCE && std::abs(currentPosY - CHARGER_Y) <= TOLERANCE) {
            std::cout << "Reached the charging station." << std::endl;
            MotorControl::stop();
            break;
        }

        if (currentPosX < CHARGER_X) {
            MotorControl::turnRight();
            usleep(500000);
            MotorControl::stop();
            MotorControl::forward();
        } else if (currentPosX > CHARGER_X) {
            MotorControl::turnLeft();
            usleep(500000);
            MotorControl::stop();
            MotorControl::forward();
        }

        if (currentPosY < CHARGER_Y) {
            MotorControl::forward();
        } else if (currentPosY > CHARGER_Y) {
            MotorControl::backward();
        }

        usleep(500000);
        MotorControl::stop();

        if (stopProgram.load() || userStopMovement.load() || !navigateToCharger.load()) {
            MotorControl::stop();
            break;
        }
    }
}

int main() {
    // Register signal handler
    signal(SIGINT, signalHandler);

    std::thread gpsWifiThread(gps_wifi_thread);

    // Initialize motor control
    MotorControl::setup();
    std::cout << "Motor controller set up done" << std::endl;

    int photoCounter = 0;
    std::thread camThread(camera_thread, std::ref(photoCounter));
    std::thread userInputThread(user_input_thread);

    UltrasonicSensor frontSensorL("Front-left", 26, 24);
    UltrasonicSensor frontSensorR("Front-right", 20, 21);
    UltrasonicSensor rightSensor("Right", 22, 27);
    UltrasonicSensor leftSensor("Left", 18, 17);

    ServoControl arm;
    arm.setup();
    std::cout << "arm set up done" << std::endl;

    while (!stopProgram.load()) {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [] { return !stopMovement.load() || stopProgram.load(); });

      if (stopProgram.load()) MotorControl::stop();

      if (navigateToCharger.load()) 
      {
        navigate_to_charger();
        navigateToCharger.store(false);
      } else if (userStopMovement.load()) 
      {
         MotorControl::stop();
      } else {
        
        MotorControl::stop();
        int distanceFrontL = frontSensorL.getDistanceCm();
        int distanceFrontR = frontSensorR.getDistanceCm();
        int distanceRight = rightSensor.getDistanceCm();
        int distanceLeft = leftSensor.getDistanceCm();

        std::cout << " ------------------------------------------------------------ " << std::endl;
        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;
        std::cout << "Right Distance : " << distanceRight << " cm" << std::endl;
        std::cout << "Left Distance : " << distanceLeft << " cm" << std::endl;
        std::cout << " ------------------------------------------------------------ " << std::endl;

        bool validFrontL = (distanceFrontL != -1 && distanceFrontL < 30);
        bool validFrontR = (distanceFrontR != -1 && distanceFrontR < 30);
        bool validRight = (distanceRight != -1 && distanceRight < 20);
        bool validLeft = (distanceLeft != -1 && distanceLeft < 20);

        if (validFrontL || validFrontR) 
        {
            std::cout << "Obstacle detected. Stopping and taking a photo..." << std::endl;
            stopMovement.store(true);
            photoTaken.store(false);
            cv.notify_all();
            cv.wait(lock, [] { return photoTaken.load() || stopProgram.load(); });

            if (stopProgram.load()) MotorControl::stop();

            if (trashDetected.load()) {
                if(trashLocation=="center")
                {
                    std::cout << "Trash detected in center. Moving closer to pick it up..." << std::endl;
                    if((distanceFrontL || distanceFrontR)>=20)
                    {
                        MotorControl::forward();
                        usleep(300000); // Move forward for half second to get closer to the trash
                    }
                    MotorControl::stop();
                }else if (trashLocation == "turn left")
                {
                    std::cout << "Trash detected in left side. Moving closer to pick it up..." << std::endl;
                    MotorControl::turnLeft();
                    usleep(200000); // Move forward for half second to get closer to the trash
                    MotorControl::stop();

                }else if(trashLocation == "turn right")
                {
                    std::cout << "Trash detected in right side. Moving closer to pick it up..." << std::endl;
                    MotorControl::turnRight();
                    usleep(200000); // Move forward for half second to get closer to the trash
                    MotorControl::stop();

                }
                std::cout << "Picking up trash..." << std::endl;
                arm.open();
                sleep(1);
                arm.down();
                sleep(1);
                arm.close();
                sleep(1);
                arm.up();
                sleep(2);
                MotorControl::backward();
                usleep(1000000);
                MotorControl::turnLeft();
                usleep(500000);
            }
            else
            {
                MotorControl::backward();
                usleep(1000000);
                MotorControl::turnLeft();
                usleep(500000);
            }
            std::cout << "Resuming movement..." << std::endl;
        } else if (validRight) {
            MotorControl::turnRight();
            usleep(500000);
            MotorControl::stop();
            
        } else if (validLeft) {
            MotorControl::turnLeft();
            usleep(500000);
            MotorControl::stop();
        } else if ((validFrontL || validFrontR) && validLeft && validRight) {
            MotorControl::backward();
            usleep(1000000);
            MotorControl::turnRight();
            usleep(500000);
        } else {
            // If no valid obstacle is directly in front, move forward
            MotorControl::forward();
            std::cout << "Path is clear. Moving forward..." << std::endl;
        }

        usleep(1000000); // half second delay for general loop control
      }

    }

    std::cout << "Program terminated gracefully." << std::endl;
    

    return 0;
}
