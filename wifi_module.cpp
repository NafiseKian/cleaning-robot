#include "wifi_module.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include <Eigen/Dense>

double Localization::extractRSSIFromTcpdump(const std::string& tcpdumpOutput) {
    // Parse tcpdump output to extract RSSI value
    // This is just a placeholder implementation, you'll need to implement proper parsing
    // Example: "Signal strength (dBm): -70"
    size_t pos = tcpdumpOutput.find("Signal strength (dBm):");
    if (pos != std::string::npos) {
        std::string rssiStr = tcpdumpOutput.substr(pos + 22);
        return std::stod(rssiStr);
    }
    return 0.0; // Return 0 if RSSI value not found
}


// Constructor
Localization::Localization(const std::vector<AccessPoint>& access_points) : access_points(access_points) 
{

}

// Function to calculate distance between two points
double Localization::distance(double x1, double y1, double x2, double y2) 
{
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

// Function to estimate location using trilateration
std::pair<double, double> trilaterate(  const std::vector<std::pair<double, double>>& accessPoints,
                                        const std::vector<double>& distances,
                                        double minX, double maxX, double minY, double maxY) 
    
{

  // Check if access point and distance sizes match
  if (accessPoints.size() != distances.size()) {
    std::cerr << "Error: Access point and distance sizes don't match." << std::endl;
    return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
  }

  int numAPs = accessPoints.size();

  // Create a matrix to store equation coefficients
  Eigen::MatrixXd A(numAPs, 2);

  // Create a vector to store distance constants
  Eigen::VectorXd b(numAPs);

  // Fill the A and b matrices based on trilateration equations
  for (int i = 0; i < numAPs; ++i) {
    double x = accessPoints[i].first;
    double y = accessPoints[i].second;
    double d = distances[i];

    A(i, 0) = 2.0 * x;
    A(i, 1) = 2.0 * y;
    b(i) = pow(d, 2) - pow(x, 2) - pow(y, 2);
  }

  // Solve the linear system (Ax = b) using Eigen library
  Eigen::VectorXd solution = A.colPivHouseholderQr().solve(b);

  // Check if solution exists
  if (solution.size() != 2 || !solution.allFinite()) {
    std::cerr << "Error: No solution found for trilateration." << std::endl;
    return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
  }

  // Extract and validate estimated location
  double estimatedX = solution(0);
  double estimatedY = solution(1);

  if (estimatedX < minX || estimatedX > maxX || estimatedY < minY || estimatedY > maxY) {
    std::cerr << "Warning: Estimated location outside provided bounds." << std::endl;
  }

  return {estimatedX, estimatedY};
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

// Method to put WiFi interface into monitor mode
void Localization::setMonitorMode() {
    system("sudo iw dev wlan0 interface add mon0 type monitor");
}

// Method to capture WiFi packets and extract RSSI values
std::string Localization::captureWiFiPackets() {
    std::string tcpdumpCmd = "sudo tcpdump -i mon0 -e -s 256 type mgt subtype beacon -vvv 2>&1";
    FILE* pipe = popen(tcpdumpCmd.c_str(), "r");
    if (!pipe) return "Error";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);
    return result;
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