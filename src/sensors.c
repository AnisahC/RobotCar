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
#include "RGB_Sensor.h"

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
  bool* flag = ((sensor_param_t*) arg)->flag;


  //FREE STRUCT
  free(arg);
  arg = NULL;

  while(*flag){
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
  bool*       flag         = ((echo_param_t*) arg)->flag;

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

  while(*flag){
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

      //printf("Triggering...    ");
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

      //printf("Triggered\n");

      //printf("Finding start time...     ");
      time_began = clock();
      //Loop until signal ends (MANUALLY BREAK)
      while (gpioRead(pin_echo) == 0 && *flag) {
         if (clock() - time_began > MAX_TIME) {
          //printf("Signal out of range\n");
          break;
         }
      }
      time_began = clock();
      
      //printf("Start time found\n");
      //printf("Finding end time...       ");
      while(gpioRead(pin_echo) == 1 && *flag) {
        time_end = clock();
        if (time_end - time_began > MAX_TIME) {
          //printf("Signal out of range\n");
          break;
        }
      }
      //printf("Found end time\n");

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
    } */
    

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
  bool* data         = ((button_param_t*) arg)->data;
  int   pin          = ((button_param_t*) arg)->pin;
  bool  initial_state = ((button_param_t*) arg)->initial_state;

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
       printf("BUTTON DOWN!\n");
       //loop until button raised (stops button immediately changing back)
       while(gpioRead(pin)){
          printf(".");
          usleep(PERIOD_SCAN);
       }
       printf("BUTTON UP!\n");
       *data = !initial_state;
    }

    if(usleep(PERIOD_SCAN) != 0){
       printf("[!] usleep failed!\n");
       return NULL;
    }
  }


  #if(DEBUG_FLAG)
  printf("th_button(%p) TERMINATE\n", arg);
  #endif
  return NULL;
}

void* th_rgb(void* arg) {

  #if(DEBUG_FLAG)
  printf("th_rgb(%p) START\n", arg);
  #endif

  //Extract information from passed struct
  rgb_param_t* params = (rgb_param_t*) arg;
  //char* color = params->color;
  //int i2c_address = params->i2c_address;
  //int i2c_bus = params->i2c_bus;
  int* rgb_red = params->rgb_red;
  bool* flag = params->flag;

  //FREE STRUCT
  free(arg);
  arg = NULL;
  
  // Initialize Sensor
  if(TCS34725_init() < 0){
    printf("[!] RBG SENSOR INIT FAILED!\n");
    return NULL;
  }

  RGB_Val* rgb_reading = (RGB_Val*) malloc(sizeof(RGB_Val));
  
  //Sensor Reading Main Loop
  while (*flag) {

    // Read RGB data from the sensor
    getRGB(rgb_reading);
    
    /*
    printf("The color is: %s with confidence %f\n", 
            getColorName(rgb_reading), getConfidence(rgb_reading));
    */

    // If it sees red, stop the program
    if ((rgb_reading->red > 10) && (9 > rgb_reading->green) && (9 > rgb_reading->blue)) {
      *rgb_red = 1;
      *flag = 0;
    }
    // If it sees any other color, it is off the line
    else {
      *rgb_red = 0;
    }
    
    /*
    #if(DEBUG_FLAG)
    printf("RGB: %u, %u, %u HEX:#%02X%02X%02X\n",
            rgb_reading->red, rgb_reading->green, rgb_reading->blue,
            rgb_reading->red, rgb_reading->green, rgb_reading->blue);        
    #endif
    */

    // Wait before the next read
    if (usleep(PERIOD_SCAN) != 0) {
        printf("[!] usleep failed!\n");
        TCS34725_Close();
        free(rgb_reading);
        return NULL;
    }
  }
  
  // Close I2C connection
  TCS34725_Close();
  free(rgb_reading);

  #if(DEBUG_FLAG)
  printf("th_rgb(%p) TERMINATE\n", arg);
  #endif

  return NULL;
}
