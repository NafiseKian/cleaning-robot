#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <string>
#include<iostream>

class GPSModule {
public:
    GPSModule(); // Constructor
    ~GPSModule(); // Destructor

    // Function to read and parse GPS data
    bool readData(std::string& gpsData);

private:
    int gps_fd; // Serial port file descriptor
};

#endif
