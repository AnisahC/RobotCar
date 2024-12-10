/**************************************************************
* Class:: CSC-615-01 Fall 2024
* Name:: Angelo Arriaga
* Student ID:: 923069807
* Github-Name:: DeadMartyr
* Project:: Assignment 3 - Start Your Motors!
*
* File:: A03_main.c
* Description:: A demonstration of the manipulation of a Motor by
* utilizing the pigpio library and the Motor Driver Hat
**************************************************************/

#include <pigpio.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "PCA9685.h"

#define DEBUG_FLAG 1

#define LEFT              PCA_CHANNEL_5
#define LEFT_FORWARD      PCA_CHANNEL_3
#define LEFT_BACKWARD     PCA_CHANNEL_4
#define RIGHT             PCA_CHANNEL_0
#define RIGHT_FORWARD     PCA_CHANNEL_2
#define RIGHT_REVERSE     PCA_CHANNEL_1
 
#define FORWARD 1
#define REVERSE 0

#define TURN_LEFT  0
#define TURN_RIGHT 1 

#define LEFT_LINE_SENSOR 5
#define MID_LINE_SENSOR 22

int mid_line_sensor_value = -1;
int left_line_sensor_value = -1;
pthread_mutex_t lock;

// READ LINE SENSOR
void *line_sensor_thread(void *arg) {
   while (1) {
      int line_value_mid = gpioRead(MID_LINE_SENSOR);  
      int line_value_left = gpioRead(LEFT_LINE_SENSOR);
        
      // update the shared variable
      mid_line_sensor_value = line_value_mid;
      left_line_sensor_value = line_value_left;
        
      // Print the line sensor status
      if (line_value_mid == 1) {
         //printf("middle on the line.\n");
      } else {
         //printf("middle off the line.\n");
      }

      if (line_value_left == 1) {
         printf("left on the line.\n");
      } else {
         //printf("left off the line.\n");
      }
        
      usleep(100000);  // Sleep for 0.5 seconds to reduce CPU usage
    }
   return NULL;
}

// SPEED AND DIRECTION
int setMotorSpeed(uint8_t dir, uint8_t speed){

   #if(DEBUG_FLAG)
   printf("setMotorSpeed(%d,%d)\n",dir, speed);
   #endif

   //Set Duty Cycle
   switch(dir){
      case FORWARD:
         PCA9685_SetLevel(LEFT_FORWARD, 1);
         PCA9685_SetLevel(LEFT_BACKWARD, 0);
         PCA9685_SetLevel(RIGHT_FORWARD, 1);
         PCA9685_SetLevel(RIGHT_REVERSE, 0);
         break;
      case REVERSE:
         PCA9685_SetLevel(LEFT_FORWARD, 0);
         PCA9685_SetLevel(LEFT_BACKWARD, 1);
         PCA9685_SetLevel(RIGHT_FORWARD, 0);
         PCA9685_SetLevel(RIGHT_REVERSE, 1);
         break;
      default:
         printf("[!] Invalid Direction!\n");
         return -1;
   }

   //This gives a "jolt" to allow the motor to have momentum to spin at a slower speed
   for(int i=100; i>speed; i--){
      PCA9685_SetPwmDutyCycle(LEFT, i);
      PCA9685_SetPwmDutyCycle(RIGHT, i);
      usleep(1000);
   }

   //Settles on final speed
   PCA9685_SetPwmDutyCycle(LEFT, speed);
   PCA9685_SetPwmDutyCycle(RIGHT, speed);

   //Success
   return speed;
}

// TURN MOTOR
int turnMotor(uint8_t dir) {
   #if(DEBUG_FLAG)
   printf("turning motor\n",dir);
   #endif

   //Set Duty Cycle
   switch(dir) {
      case TURN_LEFT:
         printf("inside left!\n");
         PCA9685_SetLevel(LEFT_FORWARD, 0);
         PCA9685_SetLevel(LEFT_BACKWARD, 0);
         PCA9685_SetLevel(RIGHT_FORWARD, 1);
         PCA9685_SetLevel(RIGHT_REVERSE, 0);
         break;
      case TURN_RIGHT:
         printf("inside right!\n");
         PCA9685_SetLevel(LEFT_FORWARD, 1);
         PCA9685_SetLevel(LEFT_BACKWARD, 0);
         PCA9685_SetLevel(RIGHT_FORWARD, 0);
         PCA9685_SetLevel(RIGHT_REVERSE, 0);
         break;
      default:
         printf("[!] Invalid Direction!\n");
         return -1;
   }

   if(dir == TURN_LEFT) {
      printf("setting for left!\n");
      PCA9685_SetPwmDutyCycle(RIGHT, 100);
   } 
   else if(dir == TURN_RIGHT) {
      printf("setting duty cycle for right\n");
      PCA9685_SetPwmDutyCycle(LEFT, 100);
   }

   return 1;
}

// STOP MOTOR
int stopMotor(){

   PCA9685_SetPwmDutyCycle(LEFT, 0);
   PCA9685_SetPwmDutyCycle(RIGHT, 0);

   return 0;
}

/* MAIN METHOD */
int main(int argc, char* agv[]){

   //Init
   PCA9685_Init(0x40);
   PCA9685_SetPWMFreq(100);
   if(gpioInitialise() < 0){
      printf("Failed to initialize GPIO!\n");
      return -1;
   }
   
   //initialize the pins to be input
   gpioSetMode(MID_LINE_SENSOR, PI_INPUT);
   gpioSetMode(LEFT_LINE_SENSOR, PI_INPUT);
   
   //create the threads
   pthread_t mid_sensor_thread;
   pthread_t left_sensor_thread;

   pthread_create(&mid_sensor_thread, NULL, line_sensor_thread, (void*)MID_LINE_SENSOR);
   pthread_create(&left_sensor_thread, NULL, line_sensor_thread, (void*)LEFT_LINE_SENSOR);

   while(1) {
      int mid_line_value = mid_line_sensor_value;
      int left_line_value = left_line_sensor_value;

      if(mid_line_value == 1) {
         setMotorSpeed(FORWARD, 30);
      } 

      if (left_line_value == 1) {
        while(mid_line_value != 1) {
         printf("turning left");
         turnMotor(TURN_LEFT);
         }
      }
      usleep(50000);
  }

   // Destroy the mutex after use
   pthread_mutex_destroy(&lock);

   gpioTerminate();

   //Program success
   return 0;
}
