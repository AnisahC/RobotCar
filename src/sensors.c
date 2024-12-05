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

#define DEBUG_FLAG 0

void* th_sensor(void* arg){

  #if(DEBUG_FLAG)
  printf("t_sensor_line(%p)\n", arg);
  #endif

  //Get pointer to field to mutate
  int* data = ((sensor_data_t*) arg)->data;
  //Get Pin number to read from
  int pin = ((sensor_data_t*) arg)->pin;
  //Get pointer to timer
  useconds_t* time = ((sensor_data_t*) arg)->time;


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
  return NULL;
}
