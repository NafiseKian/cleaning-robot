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
    const int NUM_READINGS = 2; // Number of readings to take
    const int TIMEOUT = 500000;  // Timeout in microseconds
    int totalDistance = 0;
    int validReadings = 0;

    for (int i = 0; i < NUM_READINGS; ++i) {
        echoReceived = false;
        startTimeUs = 0;
        echoTimeUs = 0;

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

        if (echoReceived) {
            float distanceCm = (echoTimeUs * 0.0343) / 2.0; // Correct for sound speed and round trip
            totalDistance += static_cast<int>(distanceCm);
            validReadings++;
        } else {
            std::cout << name << ": Echo not received within timeout for reading " << i + 1 << "." << std::endl;
        }

        usleep(200000); // Wait 200ms between readings
    }

    if (validReadings > 0) {
        return totalDistance / validReadings;
    } else {
        std::cout << name << ": No valid readings obtained." << std::endl;
        return -1;
    }
}

void UltrasonicSensor::echoCallback(int gpio, int level, uint32_t tick, void* userdata) {
    UltrasonicSensor* sensor = reinterpret_cast<UltrasonicSensor*>(userdata);
    if (level == 1) { // Rising edge
        sensor->startTimeUs = tick;
    } else if (level == 0) { // Falling edge
        if(sensor->startTimeUs == 0) {
            return; 
        }
        // Debounce logic: Ignore any echoes that happen too quickly
        uint32_t duration = tick - sensor->startTimeUs;
        if (duration < 2000) { // Ignore echoes less than 2ms apart
            return;
        }
        sensor->echoTimeUs = duration;
        sensor->echoReceived = true;
    }
}
