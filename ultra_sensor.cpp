#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <pigpio.h>
#include <linux/gpio.h>

UltrasonicSensor::UltrasonicSensor(int echo , int trigger)
{
    if (gpioInitialise() < 0) {
          printf("set up failed \n");
        return;
    }

    gpioSetMode(echo, PI_INPUT);
    printf("echo setupped \n");
    gpioSetMode(trigger, PI_OUTPUT);
    printf("trigger setupped \n");
}

int UltrasonicSensor::getDistanceCm() 
{
    int TIMEOUT= 1000; 
    // Send a 10 microsecond pulse to trigger the sensor
    gpioTrigger(this->triggerPin, 10, PI_HIGH);
    // Wait for the echo pin to go high
    int echoLevel = gpioWaitForEdge(this->echoPin, RISING_EDGE, 10000); // Wait for up to 10 milliseconds

    if (echoLevel == TIMEOUT) {
        printf("Timeout waiting for echo signal.\n");
        return -1; // Return error value
    }

    // Measure the duration of the echo pulse
    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    echoLevel = gpioWaitForEdge(this->echoPin, FALLING_EDGE, 10000); // Wait for up to 10 milliseconds
    gettimeofday(&endTime, NULL);

    if (echoLevel == TIMEOUT) {
        printf("Timeout waiting for echo signal.\n");
        return -1; // Return error value
    }

    // Calculate the duration of the echo pulse in microseconds
    long int pulseDuration = (endTime.tv_sec - startTime.tv_sec) * 1000000L + (endTime.tv_usec - startTime.tv_usec);

    // Calculate the distance in centimeters using the speed of sound (343 m/s)
    // Distance = (duration / 2) * speed_of_sound
    float distanceCm = pulseDuration / 2.0 * 0.0343; // Convert microseconds to seconds (34300 cm/s)

    return distanceCm;
}
