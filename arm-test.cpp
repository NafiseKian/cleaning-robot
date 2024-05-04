#include <wiringPi.h>
#include <iostream>

// Constants for the GPIO pin numbers used for each servo
const int TURN_SERVO_PIN = 0;       // Replace 0 with the GPIO pin number for the turn servo
const int SHOULDER_SERVO_PIN = 1;   // Replace 1 with the GPIO pin number for the shoulder servo
const int GRAB_SERVO_PIN = 2;       // Replace 2 with the GPIO pin number for the grab servo
const int UP_DOWN_SERVO_PIN = 3;    // Replace 3 with the GPIO pin number for the up/down servo

// Function to initialize GPIO and wiringPi
void setup() {
    wiringPiSetup();  // Initialize wiringPi
    pinMode(TURN_SERVO_PIN, OUTPUT);
    pinMode(SHOULDER_SERVO_PIN, OUTPUT);
    pinMode(GRAB_SERVO_PIN, OUTPUT);
    pinMode(UP_DOWN_SERVO_PIN, OUTPUT);
}

// Function to control the turn action
void turn(int angle) {
    int pulse = map(angle, 0, 180, 50, 250);  // Map angle to pulse width range
    digitalWrite(TURN_SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(TURN_SERVO_PIN, LOW);
    delay(20 - pulse / 1000);
}

// Function to control the shoulder movement
void moveShoulder(int angle) {
    int pulse = map(angle, 0, 180, 50, 250);
    digitalWrite(SHOULDER_SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(SHOULDER_SERVO_PIN, LOW);
    delay(20 - pulse / 1000);
}

// Function to control the grabbing action
void grab(int position) {
    int pulse = map(position, 0, 180, 50, 250);
    digitalWrite(GRAB_SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(GRAB_SERVO_PIN, LOW);
    delay(20 - pulse / 1000);
}

// Function to control the up and down movement
void moveUpDown(int height) {
    int pulse = map(height, 0, 180, 50, 250);
    digitalWrite(UP_DOWN_SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(UP_DOWN_SERVO_PIN, LOW);
    delay(20 - pulse / 1000);
}

// Main function
int main() {
    setup();
    
    // Example usage
    turn(90);
    moveShoulder(45);
    grab(135);
    moveUpDown(70);

    return 0;
}

// Helper function to map values (similar to the Arduino map function)
int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
