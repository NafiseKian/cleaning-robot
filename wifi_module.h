#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <vector>
#include <string>

class Localization {
public:
    Localization();
    
    std::pair<double, double> getCurrentPositionFromWiFi();
    
    std::string captureWifiSignal();
    
    std::vector<std::pair<std::string, double>> parseIwlistOutput(const std::string& iwlistOutput) ;
    
    std::vector<std::tuple<std::string, double, double, double>>  readWiFiFingerprintFile(const std::string& filename);
    
    std::tuple<double, double> findLocation( const std::vector<std::tuple<std::string, double, double , double>>& fingerprintData,
                                              const std::vector<std::pair<std::string, double>>& observedRSSI);

private:
   
    double distance(double x1, double y1, double x2, double y2);

    std::vector<double> calculateDistanceFromRSSI(const std::vector<double>& observedRSS);

    // Methods for controlling WiFi interface and capturing packets
    void setMonitorMode();
  
};

#endif 
