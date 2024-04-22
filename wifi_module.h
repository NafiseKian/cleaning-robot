#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <vector>
#include <string>

// Struct to represent an access point with its coordinates
struct AccessPoint {
    double x;
    double y;
    std::string mac_address;
};

class Localization {
public:
    Localization(const std::vector<AccessPoint>& access_points);
    std::pair<double, double> getCurrentPositionFromWiFi();

private:
    std::vector<AccessPoint> access_points;
    
    std::pair<double, double> trilaterate(  const std::vector<std::pair<double, double>>& accessPoints,
                                        const std::vector<double>& distances,
                                        double minX, double maxX, double minY, double maxY);

    double distance(double x1, double y1, double x2, double y2);

    std::vector<double> Localization::calculateDistanceFromRSSI(const std::vector<double>& observedRSS);

    // Methods for controlling WiFi interface and capturing packets
    void setMonitorMode();
    std::string captureWiFiPackets();
};

#endif 
