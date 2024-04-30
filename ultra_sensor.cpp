#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>

UltrasonicSensor::UltrasonicSensor(int echo, int trigger) : echoPin(echo), triggerPin(trigger), startTimeUs(0), echoTimeUs(0), echoReceived(false) {
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
    const int TIMEOUT = 21000; // Timeout in microseconds
    echoReceived = false;

    // Send trigger pulse
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
        std::cout << "Echo not received within timeout." << std::endl;
        return -1;
    }

    float distanceCm = (echoTimeUs * 0.0343) / 2.0;  // Correct for sound speed and round trip
    return static_cast<int>(distanceCm);
}


void UltrasonicSensor::echoCallback(int gpio, int level, uint32_t tick, void* userdata) {
    UltrasonicSensor* sensor = reinterpret_cast<UltrasonicSensor*>(userdata);
    std::cout << "GPIO: " << gpio << " Level: " << level << " Tick: " << tick << std::endl;
    if (level == 1) { // Rising edge
        sensor->startTimeUs = tick;
        std::cout << "Start Time Set: " << sensor->startTimeUs << std::endl;
    } 
    else if (level == 0) 
    {
        if(sensor->startTimeUs == 0)
        {
            std::cout<< "falling edge detected before rising edge"<<std::endl;
            return ; 
        }
         // Falling edge
        sensor->echoTimeUs = tick - sensor->startTimeUs;
        std::cout << "Echo Time Calculated: " << sensor->echoTimeUs << std::endl;
        sensor->echoReceived = true;
    }
}


