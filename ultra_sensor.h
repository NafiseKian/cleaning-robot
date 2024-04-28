#ifndef ULTRA_SENSOR_H
#define ULTRA_SENSOR_H


class UltrasonicSensor {
public:
    UltrasonicSensor(int echo , int trigger); // Constructor
    int getDistanceCm();

private:
    int triggerPin;
    int echoPin;
};

// change to old code
#endif

