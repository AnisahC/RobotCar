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

#define DEBUG_FLAG 0

#define PIN_SENSOR_IR   13
#define PIN_SENSOR_LINE 27

#define MICROSECONDS_UNTIL_TERMINATE 4000000
#define PERIOD_DISPLAY                100000
#define PERIOD_SCAN                    25000

/* Globals for threads to update */
int val_line = -1;
int val_ir = -1;

/* Global timer to stop program naturally */
useconds_t  microsecRemaining = MICROSECONDS_UNTIL_TERMINATE;

/* MAIN METHOD */
int main(int argc, char* agv[]){

  // STEP 1: INITIALIZE
  printf("Initializing...\n");
  if(gpioInitialise()<0){
    printf("[!] Initialization of pigpio failed! Aborting!\n");
    return -1;
  }

  if( (gpioSetMode(PIN_SENSOR_LINE, PI_INPUT)<0) || 
      (gpioSetMode(PIN_SENSOR_IR, PI_INPUT)<0)){
    printf("[!] gpioSetMode failed! Aborting!\n");
    return -1;
  }

  useconds_t* microsec_remaining = malloc(sizeof(useconds_t));
  if(microsec_remaining == NULL){
    printf("[!] malloc failed!\n");
    return -1;
  }


  int* genericdata = malloc(sizeof(int));
  if(genericdata == NULL){
    printf("[!] Malloc failed!\n");
    free(microsec_remaining);
    return -1;
  }
  *genericdata = 0;

  sensor_data_t* genericstruct = malloc(sizeof(sensor_data_t));
  if(genericstruct == NULL){
    printf("[!] Malloc failed!\n");
    free(microsec_remaining);
    free(genericdata);
    return -1;
  }
  genericstruct->data = genericdata;
  genericstruct->pin  = PIN_SENSOR_IR;

  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_line,
            thread_ir,
            thread_generic;
  pthread_create(&thread_line,    NULL, t_sensor_line,NULL);
  pthread_create(&thread_ir,      NULL, t_sensor_ir,  NULL);
  pthread_create(&thread_generic, NULL, th_sensor,    genericstruct);
  genericstruct = NULL;//This will be freed by the thread, setting to NULL here for safety

  //loop while time is not
  while(microsecRemaining > 0){
    usleep(PERIOD_DISPLAY);

    #if(DEBUG_FLAG)
    printf("IR: [%d]\nLine: [%d]\n", val_ir, val_line);
    #endif

    //Read out information as specified
    if(val_line != 0)//Reversed
      {printf("ON THE LINE, ");}
    else
      {printf("OFF THE LINE, ");}
    if(val_ir == 0)
      {printf("OBSTRUCTION DETECTED!\n");}
    else
      {printf("NO OBSTRUCTION.\n");}

    printf("Generic Data! [%d]\n", *genericdata);

    //If both sensors pick up something, decrement time to naturally end program
    if(val_ir == 0 && val_line != 0){
      //useconds_t has invalid behavior when becoming negative
      //set time remaining to 0 if decrement would make it negative
      if(microsecRemaining <= PERIOD_DISPLAY)
        {microsecRemaining = 0;}
      else
        {microsecRemaining -= PERIOD_DISPLAY;}
      #if(DEBUG_FLAG)
      printf("main microsecRemaining: [%d]\n", microsecRemaining);
      #endif
    }
  }

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  if(pthread_join(thread_line, NULL) != 0){
    printf("[!] Error joining thread_line!\n");
    free(microsec_remaining);
    free(genericdata);
    return -1;
  }
  if(pthread_join(thread_ir, NULL) != 0){
    printf("[!] Error joining thread_ir!\n");
    free(microsec_remaining);
    free(genericdata);
    return -1;
  }
  if(pthread_join(thread_generic, NULL) != 0){
    printf("[!] Error joining thread_generic!\n");
    free(microsec_remaining);
    free(genericdata);
    return -1;
  }

  free(microsec_remaining);
  microsec_remaining = NULL;
  free(genericdata);
  genericdata = NULL;

  gpioTerminate();
  return 0;
}

/* Thread Function Implementations */
void* t_sensor_line(void* arg){

  while(microsecRemaining > 0){
    val_line = gpioRead(PIN_SENSOR_LINE);

    if(usleep(PERIOD_SCAN) != 0){
       printf("[!] usleep failed!\n");
       return NULL;
    }
  }

  return NULL;
}

void* t_sensor_ir(void* arg){

  while(microsecRemaining > 0){
    val_ir = gpioRead(PIN_SENSOR_IR);

    if(usleep(PERIOD_SCAN) != 0){
       printf("[!] usleep failed!\n");
       return NULL;
    }
  }

  return NULL;
}
