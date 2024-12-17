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

#include <stdbool.h>

/* DEFINES */
//Period between displaying to terminal
#define PERIOD_DISPLAY                100000
//Period between sensors updating their values
#define PERIOD_SCAN                    25000

//Constant Speed of Sound
#define SOUND_METERS_PER_SECOND       343

#define PULSE_LEN 15          // Microseconds to trigger sensor
#define SPEED_OF_SOUND 34300  // Speed of sound in cm/sec

#define MAX_TIME 8 * CLOCKS_PER_SEC / 343

//Modes for pins
#define MODE_IN   0
#define MODE_OUT  1

#define STATE_ON  1
#define STATE_OFF 0

/* Struct to hold params for most sensors (1 or 0) */
//Note that this is intended to be malloc'd as a whole struct,
//Each function will take in a SINGLE memory address pointer
//We feed in the pointer to the struct, get the relevant data
//free the parameter struct, BUT NOT THE POINTER INSIDE
typedef struct SENSOR_PARAM{
  int*        data;//address of data to change
  int         pin;//pin number to read from
  bool*       flag;//time to terminate all threads
} sensor_param_t;

/* Struct to hold Echo Sensor Data */
typedef struct ECHO_PARAM{
  double*      data;//address of data to change
  int          pin_trigger;//pin number to trigger
  int          pin_echo;//pin number to read
  bool*        flag;//time to terminate all threads
} echo_param_t;

/* Struct to hold Button Data */
typedef struct BUTTON_PARAM{
  bool*       data;//address of data to change
  int         pin;//pin number to read from
  bool        initial_state;//What the field will be set to when thread is initialized, switched when button hit
} button_param_t;

/* Struct to hold RGB Sensor Data */
typedef struct RGB_PARAM{
  //char* color;          // RBG Color read. Red str is what matters.
  //int i2c_address;      // I2C address of the RGB sensor
  //int i2c_bus;          // I2C bus to use
  int* rgb_red;   // check if rgb red is high enough (0 or 1)
  int* line_data;
  bool* flag;     // Pointer to shared timer
} rgb_param_t;

/* Prototypes for Sensor Data */
void* th_sensor    (void* arg);//Sensors that simply read 1 or 0
void* th_echo      (void* arg);//Echo Sensor specifically
void* th_button    (void* arg);//Button to HALT program
void* th_rgb       (void* arg);//RBG Sensor thread
