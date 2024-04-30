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


// Constructor
Localization::Localization(const std::vector<AccessPoint>& access_points) : access_points(access_points) 
{
}



// Function to calculate distance between two points
double Localization::distance(double x1, double y1, double x2, double y2) 
{
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}



// Function to calculate distance from RSSI
std::vector<double> Localization::calculateDistanceFromRSSI(const std::vector<double>& observedRSS) 
{

  int numAPs = observedRSS.size();
  std::vector<double> distances(numAPs);

  // Replace with actual constants (frequency, Pt)
  const double frequency = 2400; // MHz
  const double transmittedPower = 20; // dBm

  for (int i = 0; i < numAPs; ++i) {
    double RSS = observedRSS[i];
    // FSPL formula with adjustments for reference distance and Pt
    double pathLoss = (27.55 - (20 * log10(frequency)) + abs(RSS)) / 20;
    distances[i] = std::pow(10, pathLoss) * std::pow(10, transmittedPower / 10.0);
  }

  return distances;

}



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
            macAddress = line.substr(line.find(":") + 2 , 12); // Extract MAC address
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
  std::vector<std::tuple<std::string, double, double, double>> fingerprintData;
  std::ifstream fingerprintFile(filename);
  if (fingerprintFile.is_open()) {
    std::string line;
    while (std::getline(fingerprintFile, line)) {
      std::istringstream iss(line);
      std::string macAddress, rssi_str, x_str, y_str;
      double rssi, x, y;

      if (std::getline(iss, macAddress, ',') &&
          std::getline(iss, rssi_str, ',') &&
          std::getline(iss, x_str, ',') &&
          std::getline(iss, y_str)) 
      {
        try {
          rssi = std::stod(rssi_str);
          x = std::stod(x_str);
          y = std::stod(y_str);
          fingerprintData.emplace_back(macAddress, rssi, x, y);
        } catch (const std::exception& e) {
          std::cerr << "Error parsing line: " << line << std::endl;
        }
      } else {
        std::cerr << "Error parsing line: " << line << std::endl;
      }
    }
    fingerprintFile.close();
  } else {
    std::cerr << "Error: Unable to open WiFi fingerprint file for reading." << std::endl;
  }
  return fingerprintData;
}


// Function to find the best match in WiFi fingerprint data
std::tuple<double, double> Localization::findLocation( const std::vector<std::tuple<std::string, double, double>>& fingerprintData,
                                                      const std::vector<std::pair<std::string, double>>& observedRSSI) {
    // Define a similarity measure (e.g., Euclidean distance) between observed and stored RSSI values
    auto similarity = [](double rssi1, double rssi2) {
        return std::abs(rssi1 - rssi2);
    };

    // Find the best match between observed and stored RSSI values
    double minDifference = std::numeric_limits<double>::max();
    std::tuple<double, double> bestLocation = {0.0, 0.0}; // Initialize with dummy values
    for (const auto& observed : observedRSSI) {
        for (const auto& fingerprint : fingerprintData) {
            if (std::get<0>(fingerprint) == observed.first) { // Check if MAC address matches
                double difference = similarity(std::get<1>(fingerprint), observed.second);
                if (difference < minDifference) {
                    minDifference = difference;
                    bestLocation = {std::get<1>(fingerprint), std::get<2>(fingerprint)};
                    std::cout<<"The best location is ----> x="<<std::get<1>(fingerprint) <<" , y="<<std::get<2>(fingerprint)<<std::endl;
                }
                break; // Exit inner loop once a match is found
            }
        }
    }
    return bestLocation;
}



/*
// Method to get current position based on measured distances from WiFi packets
std::pair<double, double> Localization::getCurrentPositionFromWiFi() {
    setMonitorMode();
    std::string tcpdumpOutput = captureWiFiPackets();
    double rssi = extractRSSIFromTcpdump(tcpdumpOutput);
    double distance = calculateDistanceFromRSSI(rssi);
    // Convert distance to vector of distances (for trilateration)
    std::vector<double> distances(access_points.size(), distance);
    return trilaterate(distances);
}
*/