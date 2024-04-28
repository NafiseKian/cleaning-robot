#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <pigpio.h>

#define echo 20
#define  trigger 21

UltrasonicSensor::UltrasonicSensor()
{
    if (gpioInitialise() < 0) {
          printf("set up failed \n");
        return;
    }

    gpioSetMode(echo, PI_INPUT);
    printf("echo setupped \n");
    gpioSetMode(trigger, PI_OUTPUT);
    printf("enB setupped \n");
}

int UltrasonicSensor::getDistanceCm() {
    char buffer[20];
    // Send a pulse
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", trigger);
    int value_fd = open(buffer, O_WRONLY);
    if (value_fd == -1) {
        perror("gpio/value");
        exit(1);
    }
    write(value_fd, "1", 1);
    usleep(10); // Delay for 10 microseconds
    write(value_fd, "0", 1);
    close(value_fd);

    // Measure pulse duration
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", echo);
    value_fd = open(buffer, O_RDONLY);
    if (value_fd == -1) {
        perror("gpio/value");
        exit(1);
    }
    while (read(value_fd, buffer, 1) != 1 || buffer[0] == '0');
    gettimeofday(&end_time, NULL);
    while (read(value_fd, buffer, 1) != 1 || buffer[0] == '1');
    close(value_fd);

    long pulseDuration = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);

    // Calculate distance (adjust based on sensor specifications and speed of sound)
    return pulseDuration / 58; // Assuming speed of sound is 343 m/s, convert to cm
}