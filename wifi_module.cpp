#include "wifi_module.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>

#include <Eigen/Dense>




/*  wifi coordinates for ST building 
**  main door (150,50)
**  back door (0,0)
**  coffe tria door(50,50)
**  GE door(100,0)
**  st parking door(100,50)
**  lib door (50,0)
**  class 118(125,25) 
**  class 104(25,25)
**  under stairs of back door middle of back door and middle  (25,0)
**  middle of back middle and GE (75,0)
**  between main and parking (100,50)
**  between parling and coffee (75,50)
**  intersection of coridor (75,25)
*/








// Method to capture WiFi signal information and extract RSSI values
std::string Localization::captureWifiSignal() 
{
    // Run iwconfig command and capture its output
    std::string iwconfigCmd = "iwlist wlan0 scanning | grep 'Address\\|Signal level'";
    FILE* pipe = popen(iwconfigCmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to execute iwlist command." << std::endl;
        return "error";
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);

    return result;
}

// Function to parse iwlist output and extract MAC addresses and RSSI values
std::vector<std::pair<std::string, double>> Localization::parseIwlistOutput(const std::string& iwlistOutput) {
    std::vector<std::pair<std::string, double>> observedRSSI;
    std::istringstream iss(iwlistOutput);
    std::string line;
    std::string macAddress;
    std::string keyword;
    double rssi;
    while (std::getline(iss, line)) {
        if (line.find("Address") != std::string::npos) {
            macAddress = line.substr(line.find(":") + 2 , 17); // Extract MAC address
        } else if (line.find("Signal level") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string signalStr;
            lineStream >> signalStr >> signalStr >> rssi; // Extract RSSI value
            observedRSSI.push_back({macAddress, rssi});
        }
    }
    return observedRSSI;
}


std::vector<std::tuple<std::string, double, double, double>> Localization::readWiFiFingerprintFile(const std::string& filename) {
   std::vector<std::tuple<std::string, double, double, double>> fingerprints;
        std::ifstream file(filename);
        std::string line;


        while (getline(file, line)) {
            std::istringstream iss(line);
            std::string mac;
            double rssi, value1, value2;

            getline(iss, mac, ',');
            std::string temp;

            // Parse RSSI
            if (getline(iss, temp, ',')) {
                rssi = std::stod(temp);
            }

            // Parse first additional double
            if (getline(iss, temp, ',')) {
                value1 = std::stod(temp);
            }

            // Parse second additional double
            if (getline(iss, temp, ',')) {
                value2 = std::stod(temp);
            }
            
            std::cout<<"finger print data -->"<< mac<<"   "<<rssi<<"   "<<value1<<std::endl;
            fingerprints.push_back(std::make_tuple(mac, rssi, value1, value2));
        }

        file.close();
        return fingerprints;
}

// Define a function to calculate weight based on RSSI difference
auto calculateWeight = [](double rssiDifference) {
    return exp(-rssiDifference / 20); 
};


double rssiDistance(double rssi1, double rssi2) {
    return std::abs(rssi1 - rssi2);
}

std::tuple<double, double> Localization::knnLocation(const std::vector<std::tuple<std::string, double, double, double>>& fingerprintData,
                                                     const std::vector<std::pair<std::string, double>>& observedRSSI,
                                                     int k) 
{
    std::vector<std::tuple<double, double, double>> distances; // Store (distance, x, y)

    for (const auto& observed : observedRSSI) {
        for (const auto& fingerprint : fingerprintData) {
            if (std::get<0>(fingerprint) == observed.first) {
                //std::cout<<std::get<0>(fingerprint)<<"    and the received mac is -->   "<<observed.first<<std::endl;
                
                //std::cout<<"found match"<<std::endl;
                double distance = rssiDistance(std::get<1>(fingerprint), observed.second);
                //std::cout << "Distance for MAC " << observed.first << ": " << distance << std::endl;
                distances.push_back(std::make_tuple(distance, std::get<2>(fingerprint), std::get<3>(fingerprint)));
            }
        }
    }

    // Sort by distance
    std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Calculate the average of the k nearest locations
    double xSum = 0, ySum = 0;
    int count = std::min(k, static_cast<int>(distances.size()));
    for (int i = 0; i < count; i++) {
        xSum += std::get<1>(distances[i]);
        ySum += std::get<2>(distances[i]);
    }

    if (count > 0) {
        double bestX = xSum / count;
        double bestY = ySum / count;
        return {bestX, bestY};
    } else {
        std::cerr << "No valid fingerprints found for the observed RSSI values." << std::endl;
        return {0.0, 0.0};
    }

}



