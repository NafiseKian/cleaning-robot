#include "gps_module.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sstream>
#include <iostream>

#define GPS_PORT "/dev/ttyS0" 
#define BAUDRATE B9600

using namespace std;

GPSModule::GPSModule() {
    // Open serial port
    gps_fd = open(GPS_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (gps_fd == -1) {
        perror("open");
        exit(1); // Exit on error
    }

    // Configure serial port settings
    struct termios options;
    tcgetattr(gps_fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag &= ~PARENB; // Disable parity
    options.c_cflag &= ~CSTOPB; // Set 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // Set data bits to 8
    options.c_cflag &= ~CRTSCTS; // Disable hardware flow control
    options.c_cflag |= (CREAD | CLOCAL); // Enable receiver, ignore carrier
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN); // Set raw mode
    options.c_cc[VMIN] = 0;     // Non-blocking read
    options.c_cc[VTIME] = 0;
    tcsetattr(gps_fd, TCSANOW, &options);
}

GPSModule::~GPSModule() {
    close(gps_fd);
}

bool GPSModule::readData(string& gpsData) {
    char buffer[128];
    ssize_t bytes_read;

    // Read data from serial port
    do {
        bytes_read = read(gps_fd, buffer, sizeof(buffer));
    } while (bytes_read == -1 && errno == EAGAIN); // Handle non-blocking read

    if (bytes_read > 0) {
        gpsData.append(buffer, bytes_read);
        return true;
    }

    return false; // No data available
}
