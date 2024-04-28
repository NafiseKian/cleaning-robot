import RPi.GPIO as GPIO
import time

//Set up GPIO using BCM numbering
GPIO.setmode(GPIO.BCM)

//Define the GPIO pins for the servo motors
servo_pins = [18, 19, 20, 21]

//Set the frequency for PWM
frequency = 50

//Set up the GPIO pins for PWM
for pin in servo_pins:
    GPIO.setup(pin, GPIO.OUT)
//Create PWM instances for each servo motor
pwms = [GPIO.PWM(pin, frequency) for pin in servo_pins]



duty_cycle_min = 2.5  
duty_cycle_max = 12.5 

def move_servo(servo_index, angle):
    duty_cycle = duty_cycle_min + (angle / 180) * (duty_cycle_max - duty_cycle_min)
    pwms[servo_index].ChangeDutyCycle(duty_cycle)
    time.sleep(0.5) 


def move_servo1(angle):
    move_servo(0, angle)


def move_servo2(angle):
    move_servo(1, angle)


def move_servo3(angle):
    move_servo(2, angle)

def move_servo4(angle):
    move_servo(3, angle)


try:
    while True:
    
        move_servo1(0)
        move_servo2(90)
        move_servo3(180)
        move_servo4(270)
        time.sleep(1)  

except KeyboardInterrupt:

    for pwm in pwms:
        pwm.stop()
    GPIO.cleanup()
