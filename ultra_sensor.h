#ifndef ULTRA_SENSOR_H
#define ULTRA_SENSOR_H


class UltrasonicSensor {
public:
    UltrasonicSensor(); // Constructor
    int getDistanceCm();

private:
    int triggerPin;
    int echoPin;
};


#endif
