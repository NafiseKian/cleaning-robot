
#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h> // for usleep

int main() {
    int fd;
    char command = 'a';

    if ((fd = serialOpen("/dev/ttyACM0", 9600)) < 0) { // Open serial port
        std::cerr << "Unable to open serial device." << std::endl;
        return 1;
    }

    usleep(2000000); // Wait for 2 seconds for serial to stabilize

    // Send command to Arduino
    serialPutchar(fd, command);

    usleep(100000); // Wait for data to be transmitted

    serialClose(fd); // Close serial port
    return 0;
}
