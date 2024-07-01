#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>

UltrasonicSensor::UltrasonicSensor(std::string sensorName, int echo, int trigger) : name(sensorName), echoPin(echo), triggerPin(trigger), startTimeUs(0), echoTimeUs(0), echoReceived(false) {
    if (gpioInitialise() < 0) {
        std::cout << "GPIO initialization failed." << std::endl;
        return;
    }

    gpioSetMode(triggerPin, PI_OUTPUT);
    gpioSetMode(echoPin, PI_INPUT);
    gpioWrite(triggerPin, PI_LOW);
    gpioSetAlertFuncEx(echoPin, echoCallback, this);
}

UltrasonicSensor::~UltrasonicSensor() {
    gpioTerminate();
}

int UltrasonicSensor::getDistanceCm() {
    const int TIMEOUT = 300000; // Timeout in microseconds
    echoReceived = false;

    //ensures that the trigger pin is at a low level before you send the pulse
    gpioWrite(triggerPin, PI_LOW);
    usleep(2); // Settle time
    gpioWrite(triggerPin, PI_HIGH);
    usleep(10); // Trigger pulse duration
    gpioWrite(triggerPin, PI_LOW);

    // Wait for echo
    int waitTime = 0;
    while (!echoReceived && waitTime < TIMEOUT) {
        usleep(100); // Check every 100 microseconds
        waitTime += 100;
    }

    if (!echoReceived) {
        std::cout << name << ": Echo not received within timeout." << std::endl;
        return -1;
    }

    float distanceCm = (echoTimeUs * 0.0343) / 2.0;  // Correct for sound speed and round trip
    return static_cast<int>(distanceCm);
}

void UltrasonicSensor::echoCallback(int gpio, int level, uint32_t tick, void* userdata) {
    UltrasonicSensor* sensor = reinterpret_cast<UltrasonicSensor*>(userdata);
    if (level == 1) 
    { // Rising edge
        sensor->startTimeUs = tick;
    } 
    else if (level == 0) 
    {
        if(sensor->startTimeUs == 0)
        {
            std::cout<< sensor->name << ": Falling edge detected before rising edge"<<std::endl;
            return; 
        }
        // Falling edge
        sensor->echoTimeUs = tick - sensor->startTimeUs;
        sensor->echoReceived = true;
        //std::cout << sensor->name << " Distance: " << (sensor->echoTimeUs * 0.0343 / 2.0) << " cm" << std::endl;
    }
}


int main ()
{
    UltrasonicSensor frontSensorL("Front-left", 26, 24);
    UltrasonicSensor frontSensorR("Front-right", 20, 3);
    UltrasonicSensor rightSensor("Right", 22, 27);
    UltrasonicSensor leftSensor("Left", 18, 17);

    int distanceFrontL = frontSensorL.getDistanceCm();
        int distanceFrontR = frontSensorR.getDistanceCm();
        int distanceRight = rightSensor.getDistanceCm();
        int distanceLeft = leftSensor.getDistanceCm();

        std::cout << " ------------------------------------------------------------ " << std::endl;
        std::cout << "Front Distance sensor left: " << distanceFrontL << " cm" << std::endl;
        std::cout << "Front Distance sensor right: " << distanceFrontR << " cm" << std::endl;
        std::cout << "Right Distance : " << distanceRight << " cm" << std::endl;
        std::cout << "Left Distance : " << distanceLeft << " cm" << std::endl;
        std::cout << " ------------------------------------------------------------ " << std::endl;

    

    return 0 ; 
}