#include "ultra_sensor.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>

UltrasonicSensor::UltrasonicSensor(int triggerPin, int echoPin) : triggerPin(triggerPin), echoPin(echoPin) {
    // Export GPIO pins
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/export");
    int export_fd = open(buffer, O_WRONLY);
    if (export_fd == -1) {
        perror("gpio/export");
        exit(1);
    }
    snprintf(buffer, sizeof(buffer), "%d", triggerPin);
    write(export_fd, buffer, strlen(buffer));
    close(export_fd);

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/export");
    export_fd = open(buffer, O_WRONLY);
    if (export_fd == -1) {
        perror("gpio/export");
        exit(1);
    }
    snprintf(buffer, sizeof(buffer), "%d", echoPin);
    write(export_fd, buffer, strlen(buffer));
    close(export_fd);

    // Set trigger pin as output and echo pin as input
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", triggerPin);
    int direction_fd = open(buffer, O_WRONLY);
    if (direction_fd == -1) {
        perror("gpio/direction");
        exit(1);
    }
    write(direction_fd, "out", 3);
    close(direction_fd);

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", echoPin);
    direction_fd = open(buffer, O_WRONLY);
    if (direction_fd == -1) {
        perror("gpio/direction");
        exit(1);
    }
    write(direction_fd, "in", 2);
    close(direction_fd);
}

int UltrasonicSensor::getDistanceCm() {
    char buffer[20];
    // Send a pulse
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", triggerPin);
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

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", echoPin);
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