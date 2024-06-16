#include <pigpio.h>
#include <iostream>
#include <unistd.h>

const int servo1_pin = 2; // GPIO 2
const int servo2_pin = 3; // GPIO 3
const int servo3_pin = 4; // GPIO 4

const int MIN_PULSE_WIDTH = 500;
const int MAX_PULSE_WIDTH = 2500;

int angleToPulseWidth(int angle) {
    return (MIN_PULSE_WIDTH + (angle * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180));
}

void setServoAngle(int gpio, int angle) {
    int pulseWidth = angleToPulseWidth(angle);
    gpioServo(gpio, pulseWidth);
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "pigpio initialisation failed." << std::endl;
        return 1;
    }

    gpioSetMode(servo1_pin, PI_OUTPUT);
    gpioSetMode(servo2_pin, PI_OUTPUT);
    gpioSetMode(servo3_pin, PI_OUTPUT);

    while (true) {
        for (int pos = 0; pos <= 180; pos++) {
            setServoAngle(servo1_pin, pos);       // Move servo1 from 0 to 180 degrees
            setServoAngle(servo2_pin, 180 - pos); // Move servo2 from 180 to 0 degrees
            setServoAngle(servo3_pin, pos);       // Sweep servo3 from 0 to 180 degrees
            usleep(15000); // 15 ms delay
        }
        for (int pos = 180; pos >= 0; pos--) {
            setServoAngle(servo1_pin, pos);       // Move servo1 from 180 to 0 degrees
            setServoAngle(servo2_pin, 180 - pos); // Move servo2 from 0 to 180 degrees
            setServoAngle(servo3_pin, pos);       // Sweep servo3 from 180 to 0 degrees
            usleep(15000); // 15 ms delay
        }
    }

    gpioTerminate();
    return 0;
}
