/**
**this code is the main core of the project and it should call all modules through headers 
*/

#include <iostream>
#include "camera_module.h"

int main() 
{
    int photoCounter = 0;

    CameraModule::capturePhoto(photoCounter);

    
    return 0;
}