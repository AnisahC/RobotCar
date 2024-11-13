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

#define DEBUG_FLAG 0

#define PIN_SENSOR_IR   13
#define PIN_SENSOR_LINE 27

#define MICROSECONDS_UNTIL_TERMINATE 4000000
#define PERIOD_DISPLAY                100000
#define PERIOD_SCAN                    25000

/* Prototype Signatures for Sensors */
void* t_sensor_line(void* arg);
void* t_sensor_ir(void* arg);

/* Global Timer */
useconds_t microsecRemaining = MICROSECONDS_UNTIL_TERMINATE; 
