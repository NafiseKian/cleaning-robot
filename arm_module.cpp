#include "arm_module.h"

void ArmControl::setup() {
    servo1.attach(2); //right hand 
    servo2.attach(3); //left hand
    servo3.attach(4); //big servo
}

void ArmControl::open() {
    servo1.write(160);
    servo2.write(95);
}

void ArmControl::close() {
    servo1.write(160);
    servo2.write(0);
}

void ArmControl::down() {
    servo3.write(40);
}

void ArmControl::up() {
    servo3.write(60);
}
