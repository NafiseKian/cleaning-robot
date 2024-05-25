#include <Python.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cmath>
#include <unistd.h>
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

void detectObjects() {
    // Set the PYTHONHOME and PYTHONPATH environment variables
    setenv("PYTHONHOME", "/home/ciuteam/cleaningrobot/tf-env", 1);
    setenv("PYTHONPATH", "/home/ciuteam/cleaningrobot/tf-env/lib/python3.11/site-packages", 1);

    // Initialize Python interpreter
    Py_Initialize();

    // Print a message indicating Python initialization
    std::cout << "Py Initialized is called" << std::endl;

    // Print Python version and executable for debugging
    PyObject* sys = PyImport_ImportModule("sys");
    PyObject* version = PyObject_GetAttrString(sys, "version");
    PyObject* executable = PyObject_GetAttrString(sys, "executable");
    std::cout << "Python version: " << PyUnicode_AsUTF8(version) << std::endl;
    std::cout << "Python executable: " << PyUnicode_AsUTF8(executable) << std::endl;
    Py_DECREF(version);
    Py_DECREF(executable);
    Py_DECREF(sys);

    // Set PYTHONPATH to the current directory
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_DecodeFSDefault("."));

    // Import necessary modules
    PyObject* pName = PyUnicode_DecodeFSDefault("trial");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        // Get the function
        PyObject* pFunc = PyObject_GetAttrString(pModule, "detect_objects");

        if (pFunc && PyCallable_Check(pFunc)) {
            // Create a list of class names
            PyObject* classNames = PyList_New(2);
            PyList_SetItem(classNames, 0, PyUnicode_FromString("not trash"));
            PyList_SetItem(classNames, 1, PyUnicode_FromString("trash"));

            // Call the Python function with arguments
            PyObject* pArgs = PyTuple_Pack(4, 
                PyUnicode_FromString("/home/ciuteam/cleaningrobot/cleaning-robot/images"),  // Image directory
                PyUnicode_FromString("/home/ciuteam/cleaningrobot/cleaning-robot/output"), // Output directory
                PyUnicode_FromString("/home/ciuteam/cleaningrobot/cleaning-robot/epoch_054.pt"), // Weights path
                classNames // List of class names
            );

            if (pArgs != NULL) {
                PyObject* pResult = PyObject_CallObject(pFunc, pArgs);

                // Check for errors
                if (pResult != NULL) {
                    // Do something with the result if needed
                    Py_DECREF(pResult);
                } else {
                    PyErr_Print();
                }

                // Clean up
                Py_DECREF(pArgs);
            } else {
                PyErr_Print();
                std::cerr << "Failed to create argument tuple\n";
            }

            Py_DECREF(pFunc);
        } else {
            if (PyErr_Occurred()) PyErr_Print();
            std::cerr << "Cannot find function 'detect_objects'\n";
        }

        // Clean up
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        std::cerr << "Failed to load Python module 'trial'\n";
    }

    // Finalize Python interpreter
    Py_Finalize();
}

void camera_thread(int& some_variable) {
    detectObjects();
}

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

int main() 
{
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
        std::cout << "Right Distance : " << distanceRight << " cm" <<std::endl;
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

    gpsWifiThread.join();
    camThread.join();

    return 0;
}
