#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <vector>
#include <string>


class Localization {
public:
 
    
    std::string captureWifiSignal();
    
    std::vector<std::pair<std::string, double>> parseIwlistOutput(const std::string& iwlistOutput) ;
    
    std::vector<std::tuple<std::string, double, double, double>>  readWiFiFingerprintFile(const std::string& filename);
    
    std::tuple<double, double> knnLocation(const std::vector<std::tuple<std::string, double, double, double>>& fingerprintData,
                                                     const std::vector<std::pair<std::string, double>>& observedRSSI,
                                                     int k);

private:
 
};

#endif 
