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
#include <pthread.h>

/* Prototype Signatures for Sensors */
#include "sensors.h"
int init_sensor(pthread_t* t, int*    dest, int pin);
int init_echo  (pthread_t* t, double* dest, int pin_trigger, int pin_echo);

#define DEBUG_FLAG 1

#define PIN_SENSOR_IR   4
#define PIN_SENSOR_LINE 17
#define PIN_ECHO_DIST   20
#define PIN_ECHO_TRIG   21

#define MICROSECONDS_UNTIL_TERMINATE 4000000
#define PERIOD_DISPLAY                100000
#define PERIOD_SCAN                    25000

/* Global variables for threads to utilize */
useconds_t  	microsec_remaining = MICROSECONDS_UNTIL_TERMINATE;
int             data_ir = -1;
int             data_line = -1;
double          data_dist = -1;

/* MAIN METHOD */
int main(int argc, char* agv[]){

  // STEP 1: INITIALIZE
  printf("Initializing...\n");
  if(gpioInitialise()<0){
    printf("[!] Initialization of pigpio failed! Aborting!\n");
    return -1;
  }

  if( (gpioSetMode(PIN_SENSOR_LINE, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_SENSOR_IR, PI_INPUT)   < 0)){
    printf("[!] gpioSetMode failed! Aborting!\n");
    return -1;
  }


  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_line,
            thread_ir,
            thread_echo;

  if( (init_sensor(&thread_ir,   &data_ir,   PIN_SENSOR_IR)                < 0) ||
      (init_sensor(&thread_line, &data_line, PIN_SENSOR_LINE)              < 0) ||
      (init_echo  (&thread_echo, &data_dist, PIN_ECHO_TRIG,  PIN_ECHO_DIST)< 0) ){

     printf("[!] FAILED TO INITIALIZE SENSORS!\n");
     return -1;
  }

  //loop while time is not
  while(microsec_remaining > 0){
    usleep(PERIOD_DISPLAY);

    //Read out information as specified
    if(data_line != 0)//Reversed
      {printf("ON THE LINE, ");}
    else
      {printf("OFF THE LINE, ");}
    if(data_ir == 0)
      {printf("OBSTRUCTION DETECTED!\n");}
    else
      {printf("NO OBSTRUCTION.\n");}

    printf("DISTANCE: [%f m]\n", data_dist);

    //If both sensors pick up something, decrement time to naturally end program
    if(data_ir == 0 && data_line != 0){
      //useconds_t has invalid behavior when becoming negative
      //set time remaining to 0 if decrement would make it negative
      if(microsec_remaining <= PERIOD_DISPLAY)
        {microsec_remaining = 0;}
      else
        {microsec_remaining -= PERIOD_DISPLAY;}
      #if(DEBUG_FLAG)
      printf("main microsec_remaining: [%d]\n", microsec_remaining);
      #endif
    }
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
