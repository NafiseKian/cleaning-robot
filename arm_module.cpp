#include "arm_module.h"
#include <iostream>
#include <unistd.h> 

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
    gpioServo(servo1_pin, 900); // Adjust these values as per your servo specifications
    gpioServo(servo2_pin, 1200);
}

void ServoControl::close() {
    gpioServo(servo1_pin, 1520);
    gpioServo(servo2_pin, 500); // Min pulse width
}

void ServoControl::down() {
    gpioServo(servo3_pin, 1700); // Adjust to appropriate pulse width
}

void ServoControl::up() {
    gpioServo(servo3_pin, 500); // Adjust to appropriate pulse width
}


