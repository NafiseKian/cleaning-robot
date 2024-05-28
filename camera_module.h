
#ifndef CAMERAMODULE_H
#define CAMERAMODULE_H

#include <string>

class CameraModule {
public:
    static std::string capturePhoto(int& photoCounter);
};

#endif // CAMERAMODULE_H
