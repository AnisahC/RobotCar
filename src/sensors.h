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
 * File:: sensors.h
 * Description:: The file which defines the functions used for each sensor
 *
 * *************/

/* DEFINES */
//Period between displaying to terminal
#define PERIOD_DISPLAY                100000
//Period between sensors updating their values
#define PERIOD_SCAN                    25000

/* Struct to hold all sensor data */
//Note that this is intended to be malloc'd as a whole struct,
//Each function will take in a SINGLE memory address pointer
//We feed in the pointer to the struct, get the relevant data
//free the parameter struct, BUT NOT THE POINTER INSIDE
typedef struct SENSOR_DATA{
  int* data;//address of data to change
  int  pin;//pin number to read from
} sensor_data_t;

/* Prototypes for Sensor Data */
void* t_sensor_line(void* arg);
void* t_sensor_ir  (void* arg);

