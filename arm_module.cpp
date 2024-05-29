#include "arm_module.h"

#include <iostream>

void ServoControl::setup() {
    if (gpioInitialise() < 0) {
        std::cerr << "pigpio initialization failed." << std::endl;
        return;
    }

    gpioSetMode(servo1_pin, PI_OUTPUT);
    gpioSetMode(servo2_pin, PI_OUTPUT);
    gpioSetMode(servo3_pin, PI_OUTPUT);
}

void ServoControl::open() {
    gpioServo(servo1_pin, 1600); // Adjust these values as per your servo specifications
    gpioServo(servo2_pin, 950);
}

void ServoControl::close() {
    gpioServo(servo1_pin, 1600);
    gpioServo(servo2_pin, 0);
}

void ServoControl::down() {
    gpioServo(servo3_pin, 100);
}

void ServoControl::up() {
    gpioServo(servo3_pin, 100);
}


int main()
{
    ServoControl arm;
    arm.setup();
    std::cout << "arm set up done" << std::endl;
    arm.down();
}