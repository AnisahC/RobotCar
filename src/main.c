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
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

/* Prototype Signatures for Sensors */
#include "sensors.h"
int init_sensor(pthread_t* t, int*    dest, int pin);
int init_echo  (pthread_t* t, double* dest, int pin_trigger, int pin_echo);

#include "PCA9685.h"


#define DEBUG_FLAG 1

#define PIN_SENSOR_LINE_R          17
#define PIN_SENSOR_LINE_L          5
#define PIN_SENSOR_LINE_M          22
#define PIN_SENSOR_ECHO_F_TRIGGER  21
#define PIN_SENSOR_ECHO_B_TRIGGER  24
#define PIN_SENSOR_ECHO_F_ECHO     20
#define PIN_SENSOR_ECHO_B_ECHO     23

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

double          data_echoF = -1;
double          data_echoB = -1;

int turning = 0;
int looping = 1;
int found_obstacle = 0;

void handleStop(int signal){
    printf("Stopped Loop\n");
    looping = 0;
}

/* MAIN METHOD */
int main(int argc, char* agv[]){

  // STEP 1: INITIALIZE
  printf("Initializing...\n");
  if(gpioInitialise()<0){
    printf("[!] Initialization of pigpio failed! Aborting!\n");
    return -1;
  }

  if( (gpioSetMode(PIN_SENSOR_LINE_L, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_R, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_LINE_M, PI_INPUT) < 0)  ){
    printf("[!] gpioSetMode failed! Aborting!\n");
    return -1;
  }


  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_lineR,
            thread_lineL,
            thread_lineM,
            thread_echoF,
            thread_echoB;

  if( (init_sensor(&thread_lineL, &data_lineL, PIN_SENSOR_LINE_L) < 0) ||
      (init_sensor(&thread_lineR, &data_lineR, PIN_SENSOR_LINE_R) < 0) ||
      (init_sensor(&thread_lineM, &data_lineM, PIN_SENSOR_LINE_M) < 0) ||
      (init_echo  (&thread_echoF, &data_echoF, PIN_SENSOR_ECHO_F_TRIGGER,  PIN_SENSOR_ECHO_F_ECHO) < 0) ||
      (init_echo  (&thread_echoB, &data_echoB, PIN_SENSOR_ECHO_B_TRIGGER,  PIN_SENSOR_ECHO_B_ECHO) < 0)){

     printf("[!] FAILED TO INITIALIZE SENSORS!\n");
     return -1;
  }

  //loop while time is not
  while(looping){//off line
    if (signal(SIGTSTP,handleStop) == SIG_ERR){
      break;
    }

    if(turning){
      if ((found_obstacle != 1) && (data_echoB*100 > MIN_DISTANCE) && data_echoB*100 < MAX_DISTANCE)){
        found_obstacle = 1;
      }
      if(found_obstacle != 1){
        printf("Continue 0 point turn\n");
        continue;
      }
      if (found_obstacle && ((data_echoB*100 < MIN_DISTANCE) && data_echoB*100 > MAX_DISTANCE)){
        printf("[ECHO] past obstacle, go straight \n");
        turning = 0;
        found_obstacle = 0;
        continue;
      }
      if(data_lineM != 0){
        printf("found line while turning, turn opposite of sensor side\n");
      }
    }else{//on line
      if (data_echoF > 100){
        printf("[ECHO] obstacle detected %f\n",data_echoF);
        turning = 1;
        continue;
      }
      if(data_lineM != 0){
        printf("continue forward\n");
        continue;
      }
      if(data_lineR != 0){
        printf("right sensor, turn left\n");
      }
      if(data_lineL != 0){
        printf("left sensor, turn right\n");
      }
    }
    gpioDelay(100000);
  }

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  if(pthread_join(thread_line, NULL) != 0){
    printf("[!] Error joining thread_line!\n");
    return -1;
  }
  if(pthread_join(thread_ir, NULL) != 0){
    printf("[!] Error joining thread_ir!\n");
    return -1;
  }
  if(pthread_join(thread_echo, NULL) != 0){
    printf("[!] Error joining thread_echo!\n");
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
