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

  char    started_reading;
  clock_t time_began;
  clock_t time_end;
  clock_t time_elapsed;

  while(*time_main > 0){
    double results[9];
    
    // Get 9 readings
    for (int i=0; i < 9; i++) {
      started_reading = 0;
      time_began      = 0;
      time_end        = 0;
      time_elapsed    = 0;

      // Confirm trigger pin is off
      gpioWrite(pin_trigger, 0);
      usleep(2);

      // Begin trigger
      if(gpioWrite(pin_trigger, 1)){
        printf("[!] Error starting trigger pin [%d]!\n", pin_trigger);
      }

      //Keep pulse for 15 microseconds
      usleep(PULSE_LEN);

      // End trigger
      if(gpioWrite(pin_trigger, 0)){
        printf("[!] Error ending trigger pin [%d]!\n", pin_trigger);
      }

      time_began = clock();
      //Loop until signal ends (MANUALLY BREAK)
      while (gpioRead(pin_echo) == 0 && *time_main > 0) {
        time_began = clock();
      }

      while(gpioRead(pin_echo) == 1 && *time_main > 0) {
        time_end = clock();
      }

      time_elapsed = time_end - time_began;

      results[i] = ((double) time_elapsed/CLOCKS_PER_SEC) * SPEED_OF_SOUND / 2;  
    }
    
    // Sort the results
    for (int i = 0; i < 9 - 1; i++) {
        for (int j = 0; j < 9 - i - 1; j++) {
            if (results[j] > results[j + 1]) {
                // Swap the elements
                double temp = results[j];
                results[j] = results[j + 1];
                results[j + 1] = temp;
            }
        }
    }

    // Get median result
    *data = results[4];

    // Print statement for testing
    /*
    if (pin_trigger == 21) {
      printf("Pin: [%d],    Distance: [%.2f cm]\n", pin_trigger, *data);
      for (int i=0; i<9; i++) {
        printf("%.2f, ", results[i]);
      }
      printf("\n");
    }
    */

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
