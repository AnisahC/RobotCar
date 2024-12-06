/***************
 *
 * Class:: CSC-615-01 Fall 2024
 * Names:: Anisah Chowdhury
 *         Angelo Arriaga
 *         Owen Meyer
 *         Austin Ng
 *         Citlalin Hernandez
 *
 * Github-Name:: DeadMartyr
 * Project:: Term Project
 *
 * File:: main.c
 * Description:: The main file of the final term project
 *
 * *************/

#include "../lib/PCA9685/PCA9685.h"       
#include "../lib/Config/sysfs_gpio.h"     
#include "../lib/Config/DEV_Config.h"     
#include "../lib/Config/Debug.h"          
#include <pigpio.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#define PWM_LEFT             PCA_CHANNEL_5
#define PWM_LEFT_FORWARD     PCA_CHANNEL_3
#define PWM_LEFT_BACKWARD    PCA_CHANNEL_4
#define PWM_RIGHT            PCA_CHANNEL_0
#define PWM_RIGHT_FORWARD    PCA_CHANNEL_1
#define PWM_RIGHT_BACKWARD   PCA_CHANNEL_2
 
#define LEFT  0
#define RIGHT 1
#define FORWARD 1
#define BACKWARD 0

// Function to set motor direction and speed
#define PIN_SENSOR_LINE_R          17
#define PIN_SENSOR_LINE_L          5
#define PIN_SENSOR_LINE_M          22
#define PIN_SENSOR_ECHO_F_TRIGGER  21
#define PIN_SENSOR_ECHO_B_TRIGGER  24
#define PIN_SENSOR_ECHO_F_ECHO     20
#define PIN_SENSOR_ECHO_B_ECHO     23

void setMotorDirectionSpeed(int motor, int direction, int speed)
{
    // Print the inputs for debugging
    printf("Motor: %d, Direction: %d, Speed: %d\n", motor, direction, speed);

    if (motor == LEFT) {
        PCA9685_SetLevel(PWM_LEFT, speed); // Set speed for left motor
        if (direction == FORWARD) {
            PCA9685_SetLevel(PWM_LEFT_FORWARD, 4095);  // Set forward direction (full speed)
            PCA9685_SetLevel(PWM_LEFT_BACKWARD, 0);    // Ensure backward is off
        } else if (direction == BACKWARD) {
            PCA9685_SetLevel(PWM_LEFT_BACKWARD, 4095); // Set backward direction (full speed)
            PCA9685_SetLevel(PWM_LEFT_FORWARD, 0);     // Ensure forward is off
        }
    } 
    else if (motor == RIGHT) {
        PCA9685_SetLevel(PWM_RIGHT, speed); // Set speed for right motor
        if (direction == FORWARD) {
            PCA9685_SetLevel(PWM_RIGHT_FORWARD, 4095); // Set forward direction (full speed)
            PCA9685_SetLevel(PWM_RIGHT_BACKWARD, 0);   // Ensure backward is off
        } else if (direction == BACKWARD) {
            PCA9685_SetLevel(PWM_RIGHT_BACKWARD, 4095); // Set backward direction (full speed)
            PCA9685_SetLevel(PWM_RIGHT_FORWARD, 0);     // Ensure forward is off
        }
    }
}

int main() {
    // Initialize PCA9685 with I2C address 0x40
    int init_status = PCA9685_Init(0x40);
    
    // Check if initialization was successful
    if (init_status != 0) {
        printf("Initialization failed. Exiting...\n");
        return -1;  // Exit with an error code
    } else {
        printf("PCA9685 successfully initialized.\n");
    }

    
    // Run both motors forward at speed 100
    printf("Running both motors forward at speed 100\n");
    setMotorDirectionSpeed(LEFT, FORWARD, 100);  // Left motor forward at speed 100
    setMotorDirectionSpeed(RIGHT, FORWARD, 100); // Right motor forward at speed 100
    
    // Wait for some time (e.g., 2 seconds)
    sleep(2);

    // Make only the left motor run forward at speed 100
    printf("Running left motor forward at speed 100\n");
    setMotorDirectionSpeed(LEFT, FORWARD, 100);  // Left motor forward
    setMotorDirectionSpeed(RIGHT, BACKWARD, 0);  // Stop right motor
    
    // Wait for some time (e.g., 2 seconds)
    DEV_Delay_ms(2000);

    // Make only the left motor run backward at speed 100
    printf("Running left motor backward at speed 100\n");
    setMotorDirectionSpeed(LEFT, BACKWARD, 100); // Left motor backward
    setMotorDirectionSpeed(RIGHT, BACKWARD, 0);  // Stop right motor
    
    // Wait for some time (e.g., 2 seconds)
    sleep(2);

    // Make only the right motor run forward at speed 100
    printf("Running right motor forward at speed 100\n");
    setMotorDirectionSpeed(LEFT, BACKWARD, 0);  // Stop left motor
    setMotorDirectionSpeed(RIGHT, FORWARD, 100); // Right motor forward
    
    // Wait for some time (e.g., 2 seconds)
    sleep(2);

    // Make only the right motor run backward at speed 100
    printf("Running right motor backward at speed 100\n");
    setMotorDirectionSpeed(LEFT, BACKWARD, 0);  // Stop left motor
    setMotorDirectionSpeed(RIGHT, BACKWARD, 100); // Right motor backward
    
    // Wait for some time (e.g., 2 seconds)
    sleep(2);

    // Stop both motors
    printf("Stopping both motors\n");
    setMotorDirectionSpeed(LEFT, BACKWARD, 0);  // Stop left motor
    setMotorDirectionSpeed(RIGHT, BACKWARD, 0); // Stop right motor

    return 0;
}