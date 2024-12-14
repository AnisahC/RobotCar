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
 * File:: sensors.c
 * Description:: The file implementing the functions used to read data from sensors
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

#define DEBUG_FLAG 1

void* th_sensor(void* arg){

  #if(DEBUG_FLAG)
  printf("th_sensor(%p) START\n", arg);
  #endif

  //Get pointer to field to mutate
  int* data = ((sensor_param_t*) arg)->data;
  //Get Pin number to read from
  int pin = ((sensor_param_t*) arg)->pin;
  //Get pointer to timer
  useconds_t* time = ((sensor_param_t*) arg)->time;


  //FREE STRUCT
  free(arg);
  arg = NULL;

  while(*time > 0){
    *data = gpioRead(pin);

    if(usleep(PERIOD_SCAN) != 0){
       printf("[!] usleep failed!\n");
       return NULL;
    }
  }

  #if(DEBUG_FLAG)
  printf("th_sensor(%p) TERMINATE\n", arg);
  #endif

  return NULL;
}

void* th_echo(void* arg){

  #if(DEBUG_FLAG)
  printf("th_echo(%p) START\n", arg);
  #endif

  //Extract information from passed struct
  double*     data         = ((echo_param_t*) arg)->data;
  int         pin_trigger  = ((echo_param_t*) arg)->pin_trigger;
  int         pin_echo     = ((echo_param_t*) arg)->pin_echo;
  useconds_t* time_main    = ((echo_param_t*) arg)->time;

  //FREE STRUCT
  free(arg);
  arg = NULL;

  //Init Pin Modes
  if(gpioSetMode(pin_trigger, MODE_OUT)||
     gpioSetMode(pin_echo,    MODE_IN )){
    printf("[!] Failed to set pin modes!\n");
    return NULL;
  }

  while(*time_main > 0){

    if(gpioWrite(pin_trigger, 1)){
      printf("[!] Error starting trigger pin [%d]!\n", pin_trigger);
    }

    //Keep pulse for 15 microseconds (15000 nanoseconds)
    usleep(15000);

    if(gpioWrite(pin_trigger, 0)){
      printf("[!] Error ending trigger pin [%d]!\n", pin_trigger);
    }

    char    started_reading = 0;
    clock_t time_began      = 0;
    clock_t time_end        = 0;

    //Loop until signal ends (MANUALLY BREAK)
    while(1){
      if( (gpioRead(pin_echo)) && (started_reading==0) ){
        time_began = clock();
        started_reading = 1;
      }
      else if( !(gpioRead(pin_echo)) && (started_reading==1) ){
        time_end = clock();
        break;
      }
    }
    clock_t time_elapsed = time_end - time_began;

    *data = (time_elapsed) * ( (double) SOUND_METERS_PER_SECOND )/( (double) CLOCKS_PER_SEC * 2);

    //printf("Distance: [%f m]\n", *data);

    //Take a break before updating distance
    if(usleep(PERIOD_SCAN) != 0){
      printf("[!] usleep failed!\n");
      return NULL;
    }
  }

  #if(DEBUG_FLAG)
  printf("th_echo(%p) TERMINATE\n", arg);
  #endif
  return NULL;
}

void* th_button(void* arg){

  #if(DEBUG_FLAG)
  printf("th_button(%p) START\n", arg);
  #endif

  //Extract information from passed struct
  char* data         = ((button_param_t*) arg)->data;
  int   pin          = ((button_param_t*) arg)->pin;
  char  initial_state = ((button_param_t*) arg)->initial_state;

  //FREE STRUCT
  free(arg);
  arg = NULL;

  //Init Pin Modes
  if(gpioSetMode(pin, MODE_IN)){
    printf("[!] Failed to set pin modes!\n");
    return NULL;
  }

  //Loop until data is no longer the initial value
  while(*data == initial_state){

    if(gpioRead(pin)){
       *data = !initial_state;
    }

    if(usleep(PERIOD_SCAN) != 0){
       printf("[!] usleep failed!\n");
       return NULL;
    }
  }


  #if(DEBUG_FLAG)
  printf("th_echo(%p) TERMINATE\n", arg);
  #endif
  return NULL;
}
