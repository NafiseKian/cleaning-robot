#ifndef ULTRA_SENSOR_H
#define ULTRA_SENSOR_H

#include <pigpio.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(int echo, int trigger); // Constructor
    ~UltrasonicSensor();                     // Destructor
    int getDistanceCm();                     // Method to get distance

private:
    int triggerPin;
    int echoPin;
    volatile long startTimeUs, echoTimeUs;
    volatile bool echoReceived;
    static void echoCallback(int gpio, int level, uint32_t tick, void* userdata);
};

#endif // ULTRA_SENSOR_H
