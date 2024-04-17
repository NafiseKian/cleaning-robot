#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define enA 5  
#define in1 7  
#define in2 8  
#define in3 9  
#define in4 10 
#define enB 6  

void setup() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        printf("Failed to open GPIO chip\n");
        return;
    }

    line = gpiod_chip_get_line(chip, enA);
    if (!line) {
        printf("Failed to get line %d\n", enA);
        gpiod_chip_close(chip);
        return;
    }

    ret = gpiod_line_request_output(line, "enA", 0);
    if (ret < 0) {
        printf("Failed to request line %d\n", enA);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return;
    }

    // Similar setup for other GPIO pins

    printf("GPIO setup done\n");

    gpiod_chip_close(chip);
}

void forward() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        printf("Failed to open GPIO chip\n");
        return;
    }

    line = gpiod_chip_get_line(chip, enA);
    if (!line) {
        printf("Failed to get line %d\n", enA);
        gpiod_chip_close(chip);
        return;
    }

    ret = gpiod_line_set_value(line, 1);
    if (ret < 0) {
        printf("Failed to set line %d\n", enA);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return;
    }

    // Similar setup for other GPIO pins

    printf("Motors moving forward\n");

    gpiod_chip_close(chip);
}

void stop() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        printf("Failed to open GPIO chip\n");
        return;
    }

    line = gpiod_chip_get_line(chip, enA);
    if (!line) {
        printf("Failed to get line %d\n", enA);
        gpiod_chip_close(chip);
        return;
    }

    ret = gpiod_line_set_value(line, 0);
    if (ret < 0) {
        printf("Failed to set line %d\n", enA);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return;
    }

    // Similar setup for other GPIO pins

    printf("Motors stopped\n");

    gpiod_chip_close(chip);
}

int main() {
    // Setup GPIO
    setup();

    // Move forward for a few seconds
    forward();
    sleep(5); // Move forward for 5 seconds
    stop();

    return 0;
}
