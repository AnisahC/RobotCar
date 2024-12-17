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
int init_echo  (pthread_t* t, double* dest, int pin_trigger, int  pin_echo);
int init_button(pthread_t* t, bool*   dest, int pin,         bool initial_status);
int init_rgb   (pthread_t* t, int* rgb_red);

//Behavior prototypes
int avoid_obstacle();

#include "PCA9685.h"
#include "motors.h"

#define DEBUG_FLAG 1

// MOTOR DEFINES
// ---------------------------------------
#define LEFT              PCA_CHANNEL_5
#define LEFT_FORWARD      PCA_CHANNEL_3
#define LEFT_BACKWARD     PCA_CHANNEL_4
#define RIGHT             PCA_CHANNEL_0
#define RIGHT_FORWARD     PCA_CHANNEL_2
#define RIGHT_REVERSE     PCA_CHANNEL_1

#define FORWARD 1
#define REVERSE 0

#define TURN_LEFT  0
#define TURN_RIGHT 1

#define ON_THE_LINE  1
#define OFF_THE_LINE 0
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
bool            is_running = false;

int             data_lineR = -1;
int             data_lineL = -1;
int             data_lineM = -1;
int             data_lineIR = -1;
int             data_lineIL = -1;

int            data_rgb = 0;

double          data_echoF = -1;
double          data_echoB = -1;

int turning = 0;
int looping = 1;
int found_obstacle = 0;

/* MAIN METHOD */
int main(int argc, char* agv[]){
  double direction = 0;
  int tally = 0;

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
  //printf("Spawning threads...\n");

  pthread_t thread_lineR,
            thread_lineL,
            thread_lineM,
            thread_lineIR,
            thread_lineIL,
            thread_echoF,
            thread_echoB,
            thread_button,
            thread_rgb;

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
      (init_sensor(&thread_lineIR, &data_lineIR, PIN_SENSOR_LINE_INNER_R) < 0) ||
      (init_sensor(&thread_lineIL, &data_lineIL, PIN_SENSOR_LINE_INNER_L) < 0) ||
      (init_echo  (&thread_echoF, &data_echoF, PIN_SENSOR_ECHO_F_TRIGGER,  PIN_SENSOR_ECHO_F_ECHO) < 0) ||
      (init_echo  (&thread_echoB, &data_echoB, PIN_SENSOR_ECHO_B_TRIGGER,  PIN_SENSOR_ECHO_B_ECHO) < 0) ||
      (init_rgb   (&thread_rgb, &data_rgb) < 0)){

     printf("[!] FAILED TO INITIALIZE SENSORS!\n");
     return -1;
  }


  //loop while time is not
  while(is_running){//off line
    direction = 0;
    if (data_rgb == 1) {
        //RGB found Red
        printf("RED: [%p]\n", data_rgb);
        printf("[TERMINATE] RGB found Red\n");
        looping = 0;
        break;
    }
    if (data_echoF < 35 && data_echoF > 0){
      printf("[ECHO] obstacle detected %f\n",data_echoF);
      motor_stop();
      usleep(200000);
      avoid_obstacle();
      continue;
    }
    if(data_lineM != 0){
      tally++;
    }
    if(data_lineR != 0){
      //printf("right sensor, turn left\n");
      direction += 1;
      tally++;
    }
    if(data_lineL != 0){
      direction -= 1;
      tally++;
      //printf("left sensor, turn right\n");
    }
    if(data_lineIL != 0){
      direction -= 0.8;
      tally++;
    }
    if(data_lineIR != 0){
      direction += 0.8;
      tally++;
    }

    motor_steer(direction / tally, 80, FORWARD);
    tally = 0;


    if(data_lineR !=ON_THE_LINE && data_lineM != ON_THE_LINE &&
       data_lineL != ON_THE_LINE && data_lineIL != ON_THE_LINE &&
       data_lineIR != ON_THE_LINE) {
      motor_stop();
      usleep(250 * 1000);
      motor_steer(direction / tally, 80, REVERSE);
    }
    gpioDelay(PERIOD_DISPLAY);
  }

  microsec_remaining = 0;

  // STEP 3: TERMINATE
  printf("Terminating program...\n");

  motor_stop();

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
  if(pthread_join(thread_rgb, NULL) != 0){
    printf("[!] Error joining thread_rgb!\n");
    return -1;
  }
  else{printf("Joined thread_rgb\n");}

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

int init_rgb(pthread_t* t, int* rgb_red) {
  #if(DEBUG_FLAG)
  printf("init_rgb([%p], [%d])\n", t, rgb_red);
  #endif

  rgb_param_t* genericstruct = malloc(sizeof(rgb_param_t));

  genericstruct->rgb_red = rgb_red;
  genericstruct->flag = &is_running;

  pthread_create(t, NULL, th_rgb, genericstruct);

  //Struct will be freed by the thread above
  genericstruct = NULL;

  return 0; //Success
}

/*  Behavior Functions   */

int avoid_obstacle(){

  //Turn left immediately, stop when back sensor sees object
  while((!(data_echoB > MIN_DISTANCE && data_echoB < 40)) && is_running){
    printf("Initial left turn.\n");
    printf("Butt sensor detects: %f\n", data_echoB);
    motor_pivot(TURN_LEFT);
  }
  printf("DIFFERENT Butt sensor detects: %f\n", data_echoB);

  motor_stop();
  usleep(1*1000*1000);
  motor_set_speed(FORWARD, 50);
  usleep(100*1000);
  //While no line is found...
  while((data_lineM + data_lineIL + data_lineIR + data_lineL + data_lineR) < 3 && is_running){
    //Go forward when back sensor sees object
    if((data_echoB > MIN_DISTANCE && data_echoB < 50)){
      printf("Go Forward!\n");
      motor_set_speed(FORWARD, 50);
      usleep(50 * 1000);
    }
    else{
      printf("Turn Right!\n");
      motor_pivot(TURN_RIGHT);
      usleep(750 * 1000);
      motor_set_speed(FORWARD, 50);
      for (int i=0; i < 200; i++) {
        if (!((data_lineM + data_lineIL + data_lineIR + data_lineL + data_lineR) < 3 && is_running)) {
          i = 201;
          printf("======HIT LINE WHILE TURNING========\n");
          continue;
        }
        usleep(10 * 1000);
      }
    }
  }

  //When a line is found, drive until the left sensor sees it
  while((data_lineL == ON_THE_LINE) && is_running){
    printf("Found Line! Go Left!\n");
    motor_pivot(TURN_LEFT);
  }

  motor_stop();

  return 0;
}
