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
#include <signal.h>

/* Prototype Signatures for Sensors */
#include "sensors.h"
int init_sensor(pthread_t* t, int*    dest, int pin);
int init_echo  (pthread_t* t, double* dest, int pin_trigger, int  pin_echo);
int init_button(pthread_t* t, bool*   dest, int pin,         bool initial_status);

#include "PCA9685.h"


#define DEBUG_FLAG 1

#define PIN_SENSOR_LINE_R          17
#define PIN_SENSOR_LINE_L          5
#define PIN_SENSOR_LINE_M          22
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
bool            is_running = false;

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
      (gpioSetMode(PIN_SENSOR_LINE_M, PI_INPUT) < 0) ||
      (gpioSetMode(PIN_BUTTON, PI_INPUT) < 0)){
    printf("[!] gpioSetMode failed! Aborting!\n");
    return -1;
  }


  // STEP 2: SPAWN THREADS
  printf("Spawning threads...\n");

  pthread_t thread_lineR,
            thread_lineL,
            thread_lineM,
            thread_echoF,
            thread_echoB,
            thread_button;

  //Initialize button that will start the program
  if( init_button(&thread_button, &is_running, PIN_BUTTON, false) < 0){
    printf("[!] FAILED TO INITIALIZE BUTTON!\n");
    return -1;
  }

  //Loop until button is pressed
  printf("Please press the button on pin [%d] to start the program.\n", PIN_BUTTON);
  while(!is_running){
     usleep(PERIOD_SCAN);
  }


  //Join thread for button and remake it.
  if(pthread_join(thread_button, NULL) != 0){
    printf("[!] Error joining thread_button!\n");
    return -1;
  }
  if( init_button(&thread_button, &is_running, PIN_BUTTON, true) < 0){
    printf("[!] FAILED TO INITIALIZE BUTTON!\n");
    return -1;
  }


  //Init all sensors once button has been pressed
  printf("Starting Program...\n");
  if( (init_sensor(&thread_lineL, &data_lineL, PIN_SENSOR_LINE_L) < 0) ||
      (init_sensor(&thread_lineR, &data_lineR, PIN_SENSOR_LINE_R) < 0) ||
      (init_sensor(&thread_lineM, &data_lineM, PIN_SENSOR_LINE_M) < 0) ||
      (init_echo  (&thread_echoF, &data_echoF, PIN_SENSOR_ECHO_F_TRIGGER,  PIN_SENSOR_ECHO_F_ECHO) < 0) ||
      (init_echo  (&thread_echoB, &data_echoB, PIN_SENSOR_ECHO_B_TRIGGER,  PIN_SENSOR_ECHO_B_ECHO) < 0)){

     printf("[!] FAILED TO INITIALIZE SENSORS!\n");
     return -1;
  }




  gpioDelay(20000);
  //loop while time is not
  while(is_running){//off line
    if (signal(SIGTSTP,handleStop) == SIG_ERR){
      looping = 0;
      break;
    }

    if(gpioRead(PIN_BUTTON) > PI_LOW){
      printf("[TERMINATE] Button Pressed\n");
      looping = 0;
      break;
    }

    if(turning){
      if ((found_obstacle != 1) && (data_echoB*100 > MIN_DISTANCE) && data_echoB*100 < MAX_DISTANCE){
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
    gpioDelay(PERIOD_DISPLAY);
  }

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  if(pthread_join(thread_lineL, NULL) != 0){
    printf("[!] Error joining thread_lineL!\n");
    return -1;
  }
  else{printf("Joined thread_lineL\n");}
  if(pthread_join(thread_lineM, NULL) != 0){
    printf("[!] Error joining thread_lineM!\n");
    return -1;
  }
  else{printf("Joined thread_lineM\n");}
  if(pthread_join(thread_lineR, NULL) != 0){
    printf("[!] Error joining thread_lineR!\n");
    return -1;
  }
  else{printf("Joined thread_lineR\n");}
  if(pthread_join(thread_echoF, NULL) != 0){
    printf("[!] Error joining thread_echoF!\n");
    return -1;
  }
  else{printf("Joined thread_echoF\n");}
  if(pthread_join(thread_echoB, NULL) != 0){
    printf("[!] Error joining thread_echoB!\n");
    return -1;
  }
  else{printf("Joined thread_echoB\n");}
  if(pthread_join(thread_button, NULL) != 0){
    printf("[!] Error joining thread_button!\n");
    return -1;
  }
  else{printf("Joined thread_button\n");}

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
  genericstruct->flag = &is_running;

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
  genericstruct->flag        = &is_running;

  pthread_create(t, NULL, th_echo, genericstruct);

  //Struct will be freed by the thread above
  genericstruct = NULL;

  return 0; //Success
}

int init_button(pthread_t* t, bool* dest, int pin, bool initial_state){
  #if(DEBUG_FLAG)
  printf("init_button([%p], [%p], [%d], [%d])\n", t, dest, pin, initial_state);
  #endif

  button_param_t* genericstruct = malloc(sizeof(button_param_t));

  //Assign relevant information for thread to use
  genericstruct->data          = dest;
  genericstruct->pin           = pin;
  genericstruct->initial_state = initial_state;

  //Variable will be assigned to the inital state's value,
  //the thread will flip the value and terminate once the button is pressed
  *genericstruct->data = initial_state;

  pthread_create(t, NULL, th_button, genericstruct);

  //Struct will be freed by the thread above
  genericstruct = NULL;

  return 0;
}
