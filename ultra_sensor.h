#ifndef ULTRA_SENSOR_H
#define ULTRA_SENSOR_H

#include <string> // Include for std::string
#include <pigpio.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(std::string name, int echo, int trigger); // Updated constructor
    ~UltrasonicSensor();                     // Destructor
    int getDistanceCm();                     // Method to get distance

private:
    std::string name;  // Name of the sensor
    int triggerPin;
    int echoPin;
    volatile long startTimeUs, echoTimeUs;
    volatile bool echoReceived;
    static void echoCallback(int gpio, int level, uint32_t tick, void* userdata);
};

#endif // ULTRA_SENSOR_H
