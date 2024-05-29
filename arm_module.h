#ifndef ARM_MODULE_H
#define ARM_MODULE_H

#include <pigpio.h>

class ServoControl {
public:
    void setup();
    void open();
    void close();
    void down();
    void up();
private:
    int servo1_pin = 2; 
    int servo2_pin = 3;
    int servo3_pin = 4;
};

#endif
