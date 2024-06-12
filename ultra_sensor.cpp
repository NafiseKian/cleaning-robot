#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>

UltrasonicSensor::UltrasonicSensor(std::string sensorName, int echo, int trigger)
    : name(sensorName), echoPin(echo), triggerPin(trigger), startTimeUs(0), echoTimeUs(0), echoReceived(false) {
    if (gpioInitialise() < 0) {
        std::cout << "GPIO initialization failed." << std::endl;
        return;
    }

    gpioSetMode(triggerPin, PI_OUTPUT);
    gpioSetMode(echoPin, PI_INPUT);
    gpioWrite(triggerPin, PI_LOW);
    gpioSetAlertFuncEx(echoPin, echoCallback, this);
}

UltrasonicSensor::~UltrasonicSensor() {
    gpioTerminate();
}

int UltrasonicSensor::getDistanceCm() {
    const int TIMEOUT = 30000; // Increased timeout in microseconds
    echoReceived = false;

    // Ensure trigger pin is low before sending pulse
    gpioWrite(triggerPin, PI_LOW);
    usleep(2); // Settle time
    gpioWrite(triggerPin, PI_HIGH);
    usleep(10); // Trigger pulse duration
    gpioWrite(triggerPin, PI_LOW);

    // Wait for echo
    int waitTime = 0;
    while (!echoReceived && waitTime < TIMEOUT) {
        usleep(100); // Check every 100 microseconds
        waitTime += 100;
    }

    if (!echoReceived) {
        std::cout << name << ": Echo not received within timeout." << std::endl;
        return -1;
    }

    float distanceCm = (echoTimeUs * 0.0343) / 2.0; // Correct for sound speed and round trip
    return static_cast<int>(distanceCm);
}

void UltrasonicSensor::echoCallback(int gpio, int level, uint32_t tick, void* userdata) {
    UltrasonicSensor* sensor = reinterpret_cast<UltrasonicSensor*>(userdata);
    if (level == 1) { // Rising edge
        sensor->startTimeUs = tick;
    } else if (level == 0) { // Falling edge
        if(sensor->startTimeUs == 0) {
            std::cout << sensor->name << ": Falling edge detected before rising edge" << std::endl;
            return; 
        }
        sensor->echoTimeUs = tick - sensor->startTimeUs;
        sensor->echoReceived = true;
        std::cout << sensor->name << " Distance: " << (sensor->echoTimeUs * 0.0343 / 2.0) << " cm" << std::endl;
    }
}
