#include "wifi_module.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
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
    double rssi;
    while (std::getline(iss, line)) {
        if (line.find("Address") != std::string::npos) {
            macAddress = line.substr(line.find(":") + 2); // Extract MAC address
        } else if (line.find("Signal level") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string signalStr;
            lineStream >> signalStr >> signalStr >> rssi; // Extract RSSI value
            // Convert signal strength to dBm
            rssi = std::stod(signalStr.substr(0, signalStr.size() - 4));
            observedRSSI.push_back({macAddress, rssi});
        }
    }
    return observedRSSI;
}



std::vector<std::pair<std::string, double>>  Localization::readWiFiFingerprintFile(const std::string& filename) {
    std::vector<std::pair<std::string, double>> fingerprintData;
    std::ifstream fingerprintFile(filename);
    if (fingerprintFile.is_open()) {
        std::string macAddress;
        double rssi;
        while (fingerprintFile >> macAddress >> rssi) {
            fingerprintData.push_back({macAddress, rssi});
        }
        fingerprintFile.close();
    } else {
        std::cerr << "Error: Unable to open WiFi fingerprint file for reading." << std::endl;
    }
    return fingerprintData;
}


// Function to find the best match in WiFi fingerprint data
std::pair<double, double> Localization::findLocation(const std::vector<std::pair<std::string, double>>& fingerprintData,
                                       const std::vector<std::pair<std::string, double>>& observedRSSI) {
    // Define a similarity measure (e.g., Euclidean distance) between observed and stored RSSI values
    auto similarity = [](double rssi1, double rssi2) {
        return std::abs(rssi1 - rssi2);
    };

    // Find the best match between observed and stored RSSI values
    double minDifference = std::numeric_limits<double>::max();
    std::pair<double, double> bestLocation = {0.0, 0.0};
    for (const auto& fingerprint : fingerprintData) {
        double sumDifference = 0.0;
        for (const auto& observed : observedRSSI) {
            if (fingerprint.first == observed.first) {
                sumDifference += similarity(fingerprint.second, observed.second);
                break; // Exit inner loop once a match is found
            }
        }
        if (sumDifference < minDifference) {
            minDifference = sumDifference;
            // Here, you could use the known location of the access point for trilateration
            // For simplicity, let's just return the observed location for the matched access point
            bestLocation = {fingerprint.first, fingerprint.second};
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