#include <Servo.h>

Servo servo1;  // create servo object to control the right hand
Servo servo2;  // create servo object to control the left hand
Servo servo3;  // create servo object to control the third servo

int servo1_pin = 2; // right hand
int servo2_pin = 3; // left hand
int servo3_pin = 4; // third servo

void setup() {
  servo1.attach(servo1_pin);  // attaches the servo on pin 2 to the servo object
  servo2.attach(servo2_pin);  // attaches the servo on pin 3 to the servo object
  servo3.attach(servo3_pin);  // attaches the servo on pin 4 to the servo object
}

void loop() {
  for (int pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees in steps of 1 degree
    servo1.write(pos);  // tell servo to go to position in variable 'pos'
    servo2.write(pos);  // tell servo to go to position in variable 'pos'
    servo3.write(pos);  // tell servo to go to position in variable 'pos'
    delay(15);          // waits 15ms for the servo to reach the position
  }
  for (int pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo1.write(pos);  // tell servo to go to position in variable 'pos'
    servo2.write(pos);  // tell servo to go to position in variable 'pos'
    servo3.write(pos);  // tell servo to go to position in variable 'pos'
    delay(15);          // waits 15ms for the servo to reach the position
  }
}
