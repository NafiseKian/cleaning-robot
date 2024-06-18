#ifndef DRIVE_MODULE_H
#define DRIVE_MODULE_H

namespace MotorControl {
    void setup();
    void forward(int speed);
    void backward(int speed ) ;
    void turnRight(int speed );
    void turnLeft(int speed );
    void stop();
}

#endif
