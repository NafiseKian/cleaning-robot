
#include "drive_module.h"
#include <pigpio.h>

#define enA 5  
#define in1 7  
#define in2 8  
#define in3 9  
#define in4 10 
#define enB 6  

namespace MotorControl {

void setup() {
    if (gpioInitialise() < 0) {
          printf("set up failed \n");
        return;
    }

    gpioSetMode(enA, PI_OUTPUT);
    printf("enA setupped \n");
    gpioSetMode(in1, PI_OUTPUT);
    gpioSetMode(in2, PI_OUTPUT);
    gpioSetMode(in3, PI_OUTPUT);
    gpioSetMode(in4, PI_OUTPUT);
    gpioSetMode(enB, PI_OUTPUT);
    printf("enB setupped \n");

    gpioPWM(enA, 180);
    gpioPWM(enB, 180);
}

void forward() {
    gpioWrite(in1, PI_LOW);  
    gpioWrite(in2, PI_HIGH); 
    gpioWrite(in3, PI_HIGH); 
    gpioWrite(in4, PI_LOW);  
}

void stop() {
    gpioWrite(in1, PI_LOW);
    gpioWrite(in2, PI_LOW);
    gpioWrite(in3, PI_LOW);
    gpioWrite(in4, PI_LOW);
}

}
