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
#include <wiringPi.h>
#include <wiringSerial.h>

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

int stationSignal = -150.0 ; 
double lastDetected = 0.0 ; 

// Coordinates for the charging station
const double CHARGER_X = 10.0;
const double CHARGER_Y = 10.0;

int FBSpeed = 70 ; 
int TurnSpeed = 90 ; 

#define SOCKET_PATH "/tmp/unix_socket_example"

Localization wifi;



sp_port* initializeSerialPort(const char* portName) {
    sp_port *port;
    sp_return result = sp_get_port_by_name(portName, &port);
    if (result != SP_OK) {
        std::cerr << "Cannot find port: " << portName << std::endl;
        return nullptr;
    }

    result = sp_open(port, SP_MODE_READ_WRITE);
    if (result != SP_OK) {
        std::cerr << "Cannot open port: " << portName << std::endl;
        sp_free_port(port);
        return nullptr;
    }

    sp_set_baudrate(port, 9600);
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);
    sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE);

    return port;
}

// Function to send a command to the Arduino
bool sendCommandToArduino(sp_port* port, const std::string& command) {
    std::string cmd = command + "\n";
    sp_nonblocking_write(port, cmd.c_str(), cmd.length());
    return true;
}


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
        std::vector<std::pair<std::string, double>> observedRSSI = wifi.parseIwlistOutput(ret);
        for (const auto& observed : observedRSSI) {
                if(observed.first == "E2:E1:E1:2C:EA:73")
                {
                    std::cout<<"_________________________________we have a signal from station_______________________________"<<std::endl ;
                    stationSignal = (int)observed.second ; 
                    std::cout<<"signal value for station is ------>"<<stationSignal<<std::endl ;  
                }
        }

        std::tuple<double, double> estimatedLocation = wifi.knnLocation(fingerprintData, observedRSSI, k);
        double currentX = std::get<0>(estimatedLocation);
        double currentY = std::get<1>(estimatedLocation);
        std::cout << "Estimated location using KNN: X = " << currentX
                  << ", Y = " << currentY << std::endl;

        if (network.connectToServer()) {
            std::cout << "Connected to server successfully." << std::endl;
            network.sendData("ROBOT," + std::to_string(currentX) + "," + std::to_string(currentY)+ ","+std::to_string(lastDetected));
        } else {
            std::cout << "Failed to connect to server." << std::endl;
        }

        sleep(4);
    }
}

void camera_thread(int &photoCounter) 
{
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
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) 
    {
        perror("connect error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (!stopProgram.load()) {

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return stopMovement.load() || stopProgram.load(); });

        if (stopProgram.load()) break;

        std::string path = CameraModule::capturePhoto(photoCounter);
        std::cout << "Photo is taken." << std::endl;

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
        if (trashDetected.load()) 
        {
            std::cout << "Trash detected in the photo !" << std::endl;
            trashLocation = placeOfTrash ; 
        } 
        else 
        {
            std::cout << "No trash detected in the photo " << std::endl;
        }

        photoCounter++;

        // Signal main thread to resume movement
        photoTaken.store(true);
        stopMovement.store(false);
        cv.notify_all();
    }

    close(sockfd);
}

void user_input_thread() 
{
    std::string input;
    while (!stopProgram.load()) 
    {
        std::cin >> input;
        if (input == "s") 
        {
            std::cout<<"-----------------------        ATTENTION       --------------------"<<std::endl;
            std::cout<<"user wish to stop the robot "<<std::endl ;
            userStopMovement.store(true);
            cv.notify_all();
        } 
        else if (input == "c") 
        {
            std::cout<<"-----------------------        ATTENTION       --------------------"<<std::endl;
            std::cout<<"user wish to continue the movement "<<std::endl ;
            userStopMovement.store(false);
            cv.notify_all();
        } 
        else if (input == "ch") 
        {
            std::cout<<"-----------------------        ATTENTION       --------------------"<<std::endl;
            std::cout<<"going back to charging station "<<std::endl ;
            navigateToCharger.store(true);
            userStopMovement.store(false);
            cv.notify_all();
        }
        else if (input =="t")
        {
            std::cout<<"-----------------------        ATTENTION       --------------------"<<std::endl;
            std::cout<<"terminating the program"<<std::endl ;
            MotorControl::stop();
            stopProgram.store(true);
            cv.notify_all();
        }
        else if(input =="b")
        {
            userStopMovement.store(true);
            cv.notify_all();
            FBSpeed +=10 ; 
            TurnSpeed +=10 ; 
            std::cout<<"-----------------------       SPEED UP        --------------------"<<std::endl;
            std::cout<<"speed of motors are increased "<<std::endl ;
            userStopMovement.store(false);
            cv.notify_all();
            std::cout<<"-----------------------       ALL SET     --------------------"<<std::endl;


        }
    }
}

// Function to navigate the robot towards the charging station
void navigate_to_charger() 
{
    int initStrength = 0 ;
    
    int counter = 0 ;

    while (!stopProgram.load()) 
    {
        if (!navigateToCharger.load()) continue;

        if (stationSignal > -40) {  // Assuming signal strength closer to -30 is stronger
            std::cout << "Reached the charging station." << std::endl;
            navigateToCharger.store(false);
            MotorControl::stop();
            return;
        }
        else
        {
            counter ++ ; 
            std::cout << "Navigating towards the charging station..." << std::endl;
            MotorControl::forward(FBSpeed);
            sleep(3);
            MotorControl::stop();
            
            if(stationSignal < initStrength)
            {
                MotorControl::backward(FBSpeed);
                sleep(5);
                if(counter %2 == 0 )
                {
                    MotorControl::turnRight(TurnSpeed);
                    sleep(1);
                }
                else
                {
                    MotorControl::turnLeft(TurnSpeed);
                    sleep(1);
                }
                MotorControl::forward(FBSpeed);
                sleep(2);
                MotorControl::stop();

            }

        }

        
    }
}

int main() 
{

    int counter = 0 ;

    // Initialize serial port for Arduino
    sp_port* port = initializeSerialPort("/dev/ttyACM0");
    if (port == nullptr) 
    {
        std::cerr << "Failed to initialize serial port." << std::endl;
        return 1;
    }

    std::thread gpsWifiThread(gps_wifi_thread);


    MotorControl::setup();
    std::cout << "Motor controller set up done" << std::endl;

    int photoCounter = 0;
    std::thread camThread(camera_thread, std::ref(photoCounter));
    std::thread userInputThread(user_input_thread);

    UltrasonicSensor frontSensorL("Front-left", 26, 24);
    UltrasonicSensor frontSensorR("Front-right", 20, 3);
    UltrasonicSensor rightSensor("Right", 22, 27);
    UltrasonicSensor leftSensor("Left", 18, 17);

    //ServoControl arm;
    //arm.setup();
    //std::cout << "arm set up done" << std::endl;

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

            if (trashDetected.load()) 
            {
                lastDetected = 1.0 ;
                if(trashLocation=="center")
                {
                    std::cout << "Trash detected in center" << std::endl;
                    int FrontL = frontSensorL.getDistanceCm();
                    int FrontR = frontSensorR.getDistanceCm();
                    std::cout<<"Distance is now ----> "<<FrontL<<" and "<<FrontR<<std::endl ; 

                    if((distanceFrontL || distanceFrontR)>=20)
                    {
                        MotorControl::forward(FBSpeed);
                        usleep(400000); // Move forward for half second to get closer to the trash
                    }
                    if((distanceFrontL || distanceFrontR)<=10)
                    {
                        MotorControl::backward(FBSpeed);
                        usleep(400000); // Move backward for half second to get closer to the trash
                    }
                    MotorControl::stop();
                }else if (trashLocation == "turn left")
                {
                    std::cout << "Trash detected in left side. Moving closer to pick it up..." << std::endl;
                    MotorControl::turnLeft(TurnSpeed);
                    usleep(300000); // Move forward for half second to get closer to the trash
                    MotorControl::stop();

                }else if(trashLocation == "turn right")
                {
                    std::cout << "Trash detected in right side. Moving closer to pick it up..." << std::endl;
                    MotorControl::turnRight(TurnSpeed);
                    usleep(300000); // Move forward for half second to get closer to the trash
                    MotorControl::stop();

                }
                std::cout << "Picking up trash..." << std::endl;

                // Example command to send to Arduino
                char command = 'a';
                if (!sendCommandToArduino(port, std::string(1, command))) {
                    std::cerr << "Failed to send command to Arduino." << std::endl;
                    sp_close(port);
                    sp_free_port(port);
                    return 1;
                }

                sleep(10);
                MotorControl::backward(FBSpeed);
                usleep(1000000);
                if((counter%2)==0) MotorControl::turnLeft(TurnSpeed);
                else MotorControl::turnRight(TurnSpeed);
                usleep(500000);
                counter ++ ; 
            }
            else
            {
                lastDetected = 0.0 ;
                MotorControl::backward(FBSpeed);
                usleep(1000000);
                MotorControl::turnLeft(TurnSpeed);
                usleep(500000);
            }
            std::cout << "Resuming movement..." << std::endl;

        } else if ((validFrontL || validFrontR) && validLeft && validRight) {
            MotorControl::backward(FBSpeed);
            usleep(1000000);
            if((counter%2)==0) MotorControl::turnLeft(TurnSpeed);
            else MotorControl::turnRight(TurnSpeed);
            usleep(500000);
            counter ++ ;
        } else {
            // If no valid obstacle is directly in front, move forward
            MotorControl::forward(FBSpeed);
            std::cout << "Path is clear. Moving forward..." << std::endl;
        }

        usleep(1000000); // half second delay for general loop control
      }

    }

    sp_close(port);
    sp_free_port(port);
    MotorControl::stop();
    std::cout << "Program terminated gracefully." << std::endl;

    
    return 0;
}
