/*
** author : Nafiseh Kian
** date : 06,04,2024
*/
#include "camera_module.h"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <ctime>
#include <iomanip>

void CameraModule::capturePhoto(int& photoCounter) {
    const std::string folderPath = "../photos/";

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << folderPath << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S") << "-" << photoCounter << ".jpg";
    std::string filePath = oss.str();

    // Construct the libcamera-still command to take a photo
    std::string command = "libcamera-still -o " + filePath + " -t 1000";
    std::cout << "Capturing: " << filePath << std::endl;

    // Execute the command
    system(command.c_str());

    // Increment the counter for the next filename
    photoCounter++;
}
