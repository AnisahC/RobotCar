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


#include <pigpio.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

/* Prototype Signatures for Sensors */
#include "sensors.h"
int init_sensor(pthread_t* t, int*    dest, int pin);
int init_echo  (pthread_t* t, double* dest, int pin_trigger, int pin_echo);

#include "PCA9685.h"

#define DEBUG_FLAG 1

// MOTOR DEFINES
// ---------------------------------------
#define LEFT              PCA_CHANNEL_5
#define LEFT_FORWARD      PCA_CHANNEL_3
#define LEFT_BACKWARD     PCA_CHANNEL_4
#define RIGHT             PCA_CHANNEL_0
#define RIGHT_FORWARD     PCA_CHANNEL_2
#define RIGHT_REVERSE     PCA_CHANNEL_1

#define PIN_SENSOR_LINE_R          17
#define PIN_SENSOR_LINE_L          5
#define PIN_SENSOR_LINE_M          22
#define PIN_SENSOR_ECHO_F_TRIGGER  21
#define PIN_SENSOR_ECHO_B_TRIGGER  24
#define PIN_SENSOR_ECHO_F_ECHO     20
#define PIN_SENSOR_ECHO_B_ECHO     23

#define FORWARD 1
#define REVERSE 0

#define TURN_LEFT  0
#define TURN_RIGHT 1 
//-----------------------------------------

#define PIN_SENSOR_LINE_R          17
#define PIN_SENSOR_LINE_L          5
#define PIN_SENSOR_LINE_M          22
#define PIN_SENSOR_LINE_INNER_R    13
#define PIN_SENSOR_LINE_INNER_L    26
#define PIN_SENSOR_ECHO_F_TRIGGER  21
#define PIN_SENSOR_ECHO_B_TRIGGER  24
#define PIN_SENSOR_ECHO_F_ECHO     20
#define PIN_SENSOR_ECHO_B_ECHO     23

#define PIN_BUTTON                 25

#define MICROSECONDS_UNTIL_TERMINATE 4000000
#define PERIOD_DISPLAY                100000
#define PERIOD_SCAN                    25000

//ECHO sensor
#define MAX_DISTANCE 200
#define MIN_DISTANCE 2

/* Global variables for threads to utilize */
useconds_t  	microsec_remaining = MICROSECONDS_UNTIL_TERMINATE;
int             data_lineR = -1;
int             data_lineL = -1;
int             data_lineM = -1;
int             data_lineIR = -1;
int             data_lineIL = -1;

double          data_echoF = -1;
double          data_echoB = -1;

int turning = 0;
int looping = 1;
int found_obstacle = 0;

void handleStop(int signal){
  printf("Stopped Loop\n");
  looping = 0;
}

// SPEED AND DIRECTION
int setMotorSpeed(uint8_t dir, uint8_t speed){

  #if(DEBUG_FLAG)
    printf("setMotorSpeed(%d,%d)\n",dir, speed);
    #endif

  //Set Duty Cycle
    switch(dir){
      case FORWARD:
      printf("going foward");
        PCA9685_SetLevel(LEFT_FORWARD, 1);
        PCA9685_SetLevel(LEFT_BACKWARD, 0);
        PCA9685_SetLevel(RIGHT_FORWARD, 1);
        PCA9685_SetLevel(RIGHT_REVERSE, 0);
        break;
      case REVERSE:
        printf("going backward");
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

  //Set Duty Cycle
  switch(dir) {
      case TURN_LEFT:
        printf("turning left!\n");
        PCA9685_SetLevel(LEFT_FORWARD, 0);
        PCA9685_SetLevel(LEFT_BACKWARD, 1);
        PCA9685_SetLevel(RIGHT_FORWARD, 1);
        PCA9685_SetLevel(RIGHT_REVERSE, 0);
        break;
      case TURN_RIGHT:
        printf("turning right!\n");
        PCA9685_SetLevel(LEFT_FORWARD, 1);
        PCA9685_SetLevel(LEFT_BACKWARD, 0);
        PCA9685_SetLevel(RIGHT_FORWARD, 0);
        PCA9685_SetLevel(RIGHT_REVERSE, 1);
        break;
      default:
        printf("[!] Invalid Direction!\n");
        return -1;
  }

  PCA9685_SetPwmDutyCycle(RIGHT, 100);
  PCA9685_SetPwmDutyCycle(LEFT, 100);

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
  double direction = 0;

  // STEP 1: INITIALIZE
  printf("Initializing...\n");
  PCA9685_Init(0x40);
  PCA9685_SetPWMFreq(100);
  if(gpioInitialise()<0){
    printf("[!] Initialization of pigpio failed! Aborting!\n");
    return -1;
  }

  if( (gpioSetMode(PIN_SENSOR_LINE_L, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_R, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_M, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_INNER_R, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_INNER_L, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_BUTTON, PI_INPUT) < 0)){
    printf("[!] gpioSetMode failed! Aborting!\n");
    return -1;
  }


  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_lineR,
            thread_lineL,
            thread_lineM,
            thread_lineIR,
            thread_lineIL,
            thread_echoF,
            thread_echoB;

  if( (init_sensor(&thread_lineL, &data_lineL, PIN_SENSOR_LINE_L) < 0) ||
      (init_sensor(&thread_lineR, &data_lineR, PIN_SENSOR_LINE_R) < 0) ||
      (init_sensor(&thread_lineM, &data_lineM, PIN_SENSOR_LINE_M) < 0) ||
      (init_sensor(&thread_lineIR, &data_lineIR, PIN_SENSOR_LINE_INNER_R) < 0) ||
      (init_sensor(&thread_lineIL, &data_lineIL, PIN_SENSOR_LINE_INNER_L) < 0) ||
      (init_echo  (&thread_echoF, &data_echoF, PIN_SENSOR_ECHO_F_TRIGGER,  PIN_SENSOR_ECHO_F_ECHO) < 0) ||
      (init_echo  (&thread_echoB, &data_echoB, PIN_SENSOR_ECHO_B_TRIGGER,  PIN_SENSOR_ECHO_B_ECHO) < 0)){

     printf("[!] FAILED TO INITIALIZE SENSORS!\n");
     return -1;
  }
  while(gpioRead(PIN_BUTTON) != PI_BAD_GPIO){
    printf("level: %d\n",gpioRead(PIN_BUTTON));
    if(gpioRead(PIN_BUTTON) > PI_LOW){
      printf("[START] Button Pressed\n");
      while (gpioRead(PIN_BUTTON) == PI_HIGH){};
      break;
    }
    gpioDelay(20000);
  }
  signal(SIGTSTP,handleStop);

  gpioDelay(20000);
  //loop while time is not
  while(looping){//off line
    // if (signal(SIGTSTP,handleStop) == SIG_ERR){
    //   looping = 0;
    //   break;
    // }
    printf("IN THE LOOP------------------\n");
    //usleep(1000*1000);

    if(gpioRead(PIN_BUTTON) > PI_LOW){
      printf("[TERMINATE] Button Pressed\n");
      stopMotor();
      gpioTerminate();
      looping = 0;
      break;
    }

    if(turning){
      printf("Inside Turning\n");
      //printf("Distance is: [%f]\n", data_echoF);
      if ((found_obstacle != 1) && (data_echoB*100 > MIN_DISTANCE) && data_echoB*100 < MAX_DISTANCE){
        found_obstacle = 1;
      }
      if(found_obstacle != 1){
        //printf("Continue 0 point turn\n");
        continue;
      }
      if (found_obstacle && ((data_echoB*100 < MIN_DISTANCE) && data_echoB*100 > MAX_DISTANCE)){
        //printf("[ECHO] past obstacle, go straight \n");
        usleep(200000);
        turning = 0;
        found_obstacle = 0;
        continue;
      }
      if(data_lineM != 0){
        printf("found line while turning, turn opposite of sensor side\n");
      }
    }else{//on line
      direction = 0; /*
      if (data_echoF > 100){
        printf("[ECHO] obstacle detected %f\n",data_echoF);
        usleep(200000);
        turning = 1;
        continue;
      } */
      if(data_lineM != 0){
        setMotorSpeed(FORWARD, 100);
        //printf("continue forward\n");
        //continue;
      }
      if(data_lineR != 0){
        //printf("right sensor, turn left\n");
        direction += 1;
      }
      if(data_lineL != 0){
        direction -=1;
        //printf("left sensor, turn right\n");
      }
      if(data_lineIL != 0){
        direction -=.25;
      }
      if(data_lineIR != 0){
        direction += 0.25;
      }
      // if(direction == 0 && data_lineR == 1){
      //   //turn left
      //   direction = -3;
      // }
      printf("L: %d | IL: %d | M: %d | IR: %d | R: %d\n",data_lineL,data_lineIL,data_lineM,data_lineIR,data_lineR);
      printf("direction : %lf\n",direction);
    }

    if(direction == 0) {
      if (data_lineIL == 1 || data_lineIR == 1) {
        // forward
        setMotorSpeed(FORWARD, 10);
      }
    } else if(direction < 0) {
      turnMotor(TURN_LEFT);
      if(direction == -1) {
        gpioDelay(10000);
      } else {
        gpioDelay(500);
      }
    } else if(direction > 0) {
      turnMotor(TURN_RIGHT);
      if(direction == 1) {
        gpioDelay(10000);
      } else {
        gpioDelay(500);
      }
    }

    if(data_lineR !=1 && data_lineM != 1 && data_lineL != 1 && data_lineIL != 1 && data_lineIR != 1) {
      setMotorSpeed(REVERSE, 15);
    }



  //   if(direction < 0){
  //     //go left
  //     turnMotor(TURN_LEFT);
  //   }else if (abs(direction) >= 1) {
  //     usleep(10000000);
  //   }else if(direction > 0){
  //     //go right
  //     turnMotor(TURN_RIGHT);
  //   }else if(abs(direction) >= 0.25) {
  //     gpioDelay(50000);
  //   }else{
  //     setMotorSpeed(FORWARD, 50);
  //   }
    gpioDelay(100000);
  }
  
  
  microsec_remaining = 0;

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  if(pthread_join(thread_lineL, NULL) != 0){
    printf("[!] Error joining thread_lineL!\n");
    return -1;
  }
  if(pthread_join(thread_lineM, NULL) != 0){
    printf("[!] Error joining thread_lineM!\n");
    return -1;
  }
  if(pthread_join(thread_lineR, NULL) != 0){
    printf("[!] Error joining thread_lineR!\n");
    return -1;
  }
  if(pthread_join(thread_echoF, NULL) != 0){
    printf("[!] Error joining thread_echoF!\n");
    return -1;
  }
  if(pthread_join(thread_echoB, NULL) != 0){
    printf("[!] Error joining thread_echoB!\n");
    return -1;
  }

  gpioTerminate();
  return 0;
  
}

/* Thread Function Implementations */

//Initialize our sensor threads
// "dest" is the address of the field being written to
// "pin"  is the pin number being utilized
int init_sensor(pthread_t* t, int* dest, int pin){
  #if(DEBUG_FLAG)
  printf("init_sensor([%p], [%p], [%d])\n", t, dest, pin);
  #endif

  sensor_param_t* genericstruct = malloc(sizeof(sensor_param_t));

  genericstruct->data = dest;
  genericstruct->pin  = pin;
  genericstruct->time = &microsec_remaining;

  pthread_create(t, NULL, th_sensor, genericstruct);

  //Struct will be freed by the thread above
  genericstruct = NULL;

  return 0; //Success
}

int init_echo(pthread_t* t, double* dest, int pin_trigger, int pin_echo){
  #if(DEBUG_FLAG)
  printf("init_sensor([%p], [%p], [%d], [%d])\n", t, dest, pin_trigger, pin_echo);
  #endif

  echo_param_t* genericstruct = malloc(sizeof(echo_param_t));

  genericstruct->data        = dest;
  genericstruct->pin_trigger = pin_trigger;
  genericstruct->pin_echo    = pin_echo;
  genericstruct->time        = &microsec_remaining;

  pthread_create(t, NULL, th_echo, genericstruct);

  //Struct will be freed by the thread above
  genericstruct = NULL;

  return 0; //Success
}
