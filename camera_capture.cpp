#include <iostream>
#include <unistd.h> 
#include <sstream>
#include <ctime>
#include <iomanip>

int main() {
    const std::string folderPath = "../photos/";
    int counter = 0;

    while(true) {
        
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << folderPath << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S") << "-" << counter << ".jpg";
        std::string filePath = oss.str();

        // Construct the libcamera-still command to force the camera to take photo
        std::string command = "libcamera-still -o " + filePath + " -t 1000";
        std::cout << "Capturing: " << filePath << std::endl;

        // Execute the command
        system(command.c_str());

        // Increment the counter for the next filename
        counter++;

        sleep(2); //it means take photo each 2 seconds 
    }

    return 0;
}
