#include "arm_module.h"
#include <iostream>
#include <unistd.h>
#include <pigpio.h>

const int servo1_pin = 2; // GPIO 2
const int servo2_pin = 3; // GPIO 3
const int servo3_pin = 4; // GPIO 4

const int MIN_PULSE_WIDTH = 500;
const int MAX_PULSE_WIDTH = 2500;


int ServoControl::angleToPulseWidth(int angle) {
    return (MIN_PULSE_WIDTH + (angle * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180));
}

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
    gpioServo(servo1_pin, angleToPulseWidth(90));  // Corresponds to 90 degrees
    gpioServo(servo2_pin, angleToPulseWidth(90));   // Corresponds to 0 degrees
}

void ServoControl::close() {
    gpioServo(servo1_pin, angleToPulseWidth(0));  // Adjust these values to match your needs
    gpioServo(servo2_pin, angleToPulseWidth(180));  // Adjust these values to match your needs
}

void ServoControl::down() {
    gpioServo(servo3_pin, angleToPulseWidth(160)); // Adjust to appropriate pulse width
}

void ServoControl::up() {
    gpioServo(servo3_pin, angleToPulseWidth(70));  // Adjust to appropriate pulse width
}

int main()
{
    ServoControl arm;
    arm.setup();
    std::cout << "arm set up done" << std::endl;
    arm.open();
                sleep(2);
                arm.down();
                sleep(3);
                arm.close();
                sleep(2);
                arm.up();
                sleep(3);

    return 0 ; 
}