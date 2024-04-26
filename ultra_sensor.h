#ifndef ULTRA_SENSOR_H
#define ULTRA_SENSOR_H


class UltrasonicSensor {
public:
    UltrasonicSensor(int triggerPin, int echoPin); // Constructor
    int getDistanceCm();

private:
    int triggerPin;
    int echoPin;
};


#endif
