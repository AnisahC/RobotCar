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
int init_sensor(pthread_t* t, int* des, int pin);

#define DEBUG_FLAG 1

#define PIN_SENSOR_IR   13
#define PIN_SENSOR_LINE 27

#define MICROSECONDS_UNTIL_TERMINATE 4000000
#define PERIOD_DISPLAY                100000
#define PERIOD_SCAN                    25000

/* Globals for threads to update */
//int val_line = -1;
//int val_ir = -1;

/* Global timer to stop program naturally */
useconds_t  	microsecRemaining = MICROSECONDS_UNTIL_TERMINATE;
int             data_ir = -1;
int             data_line = -1;

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
  *microsec_remaining = MICROSECONDS_UNTIL_TERMINATE;


  
  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_line,
            thread_ir,
            thread_generic;
  //pthread_create(&thread_line,    NULL, t_sensor_line,NULL);
  //pthread_create(&thread_ir,      NULL, t_sensor_ir,  NULL);

  //Thread 1
  //int* val_ir = malloc(sizeof(int));
  //if(val_ir == NULL){
  //  printf("[!] Malloc failed!\n");
  //  free(microsec_remaining);
  //  return -1;
  //}
  // *val_ir = 0;

  //sensor_data_t* genericstruct = malloc(sizeof(sensor_data_t));
  //if(genericstruct == NULL){
  //  printf("[!] Malloc failed!\n");
  //  free(microsec_remaining);
  //  free(val_ir);
  //  return -1;
  //}
  //genericstruct->data = val_ir;
  //genericstruct->pin  = PIN_SENSOR_IR;
  //genericstruct->time = microsec_remaining;

  //pthread_create(&thread_ir, NULL, th_sensor,    genericstruct);
  //genericstruct = NULL;//This will be freed by the thread, setting to NULL here for safety
  /////////////

  //Thread 2
/*
  int* val_line = malloc(sizeof(int));
  if(val_line == NULL){
    printf("[!] Malloc failed!\n");
    free(microsec_remaining);
    return -1;
  }
  *val_line = 0;

  genericstruct = malloc(sizeof(sensor_data_t));
  if(genericstruct == NULL){
    printf("[!] Malloc failed!\n");
    free(microsec_remaining);
    free(val_line);
    return -1;
  }
  genericstruct->data = val_line;
  genericstruct->pin  = PIN_SENSOR_LINE;
  genericstruct->time = microsec_remaining;

  pthread_create(&thread_line, NULL, th_sensor,    genericstruct);
  genericstruct = NULL;//This will be freed by the thread, setting to NULL here for safety
  /////////////
*/
  //Thread 3
  if(init_sensor(&thread_generic, &data_ir, PIN_SENSOR_IR) < 0){
     printf("[!] FAILED TO INITIALIZE SENSOR!\n");
  }
  /////////////

  //loop while time is not
  while(*microsec_remaining > 0){
    usleep(PERIOD_DISPLAY);

    #if(DEBUG_FLAG)
    //printf("IR: [%d]\nLine: [%d]NEW: [%d]\n", *val_ir, *val_line, data_ir);
    printf("TEMP: [%d]\n", data_ir);
    #endif

    //Read out information as specified
    /*
    if(*val_line != 0)//Reversed
      {printf("ON THE LINE, ");}
    else
      {printf("OFF THE LINE, ");}
    if(*val_ir == 0)
      {printf("OBSTRUCTION DETECTED!\n");}
    else
      {printf("NO OBSTRUCTION.\n");}

    //If both sensors pick up something, decrement time to naturally end program
    if(*val_ir == 0 && *val_line != 0){
      //useconds_t has invalid behavior when becoming negative
      //set time remaining to 0 if decrement would make it negative
      if(*microsec_remaining <= PERIOD_DISPLAY)
        {microsecRemaining = 0; *microsec_remaining = 0;}
      else
        {microsecRemaining -= PERIOD_DISPLAY; *microsec_remaining -= PERIOD_DISPLAY;}
      #if(DEBUG_FLAG)
      printf("main microsecRemaining: [%d]\n", microsecRemaining);
      printf("main microsec_remaining: (%p)\n", microsec_remaining);
      printf("main microsec_remaining: [%d]\n", *microsec_remaining);
      #endif
    }
    */
  }

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  /*
  if(pthread_join(thread_line, NULL) != 0){
    printf("[!] Error joining thread_line!\n");
    free(microsec_remaining);
    return -1;
  }
  if(pthread_join(thread_ir, NULL) != 0){
    printf("[!] Error joining thread_ir!\n");
    free(microsec_remaining);
    return -1;
  }

  free(microsec_remaining);
  microsec_remaining = NULL;
  free(val_ir);
  val_ir = NULL;
  free(val_line);
  val_line = NULL;
  */

  gpioTerminate();
  return 0;
}

/* Thread Function Implementations */

//Initialize our sensor threads
// "dest" is the address of the field being written to
// "pin"  is the pin number being utilized
int init_sensor(pthread_t* t, int* dest, int pin){
  printf("init_sensor([%p], [%p], [%d])\n", t, dest, pin);

  sensor_data_t* genericstruct = malloc(sizeof(sensor_data_t));

/*
  sensor_data_t genericstruct = {
    .data = dest,
    .pin = pin,
    .time = &microsecRemaining
  };
*/
  pthread_create(t, NULL, th_sensor, genericstruct);

  return 0; //Success
}


/*
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
*/
