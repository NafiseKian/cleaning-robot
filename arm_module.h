#ifndef ARM_MODULE_H
#define ARM_MODULE_H

#include <Servo.h>

class ArmControl {
public:
    void setup();
    void open();
    void close();
    void down();
    void up();
private:
    Servo servo1;
    Servo servo2;
    Servo servo3;
};

#endif
