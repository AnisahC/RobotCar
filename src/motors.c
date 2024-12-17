

#include <stdlib.h>
#include "PCA9685.h"
#include "motors.h"

#define DEBUG_FLAG 1
#define JOLT_SPEED 50
#define JOLT_DELAY 100000
#define TURN_INTENSITY 1.0

#define CLOCK_DELAY 5000

/* Basic Controls */
int motor_set_direction(uint8_t dir){
  #if(DEBUG_FLAG)
  printf("motor_set_direction(%d)\n", dir);
  #endif(DEBUG_FLAG)

  switch(dir){
    case FORWARD:
      PCA9685_SetLevel(LEFT_FORWARD,   1);
      PCA9685_SetLevel(LEFT_REVERSE,   0);
      PCA9685_SetLevel(RIGHT_FORWARD,  1);
      PCA9685_SetLevel(RIGHT_REVERSE,  0);
      break;
    case REVERSE:
      PCA9685_SetLevel(LEFT_FORWARD,   0);
      PCA9685_SetLevel(LEFT_REVERSE,   1);
      PCA9685_SetLevel(RIGHT_FORWARD,  0);
      PCA9685_SetLevel(RIGHT_REVERSE,  1);
      break;
    case TURN_LEFT:
      PCA9685_SetLevel(LEFT_FORWARD,  0);
      PCA9685_SetLevel(LEFT_REVERSE,  1);
      PCA9685_SetLevel(RIGHT_FORWARD, 1);
      PCA9685_SetLevel(RIGHT_REVERSE, 0);
      break;
    case TURN_RIGHT:
      PCA9685_SetLevel(LEFT_FORWARD,  1);
      PCA9685_SetLevel(LEFT_REVERSE,  0);
      PCA9685_SetLevel(RIGHT_FORWARD, 0);
      PCA9685_SetLevel(RIGHT_REVERSE, 1);
      break;

    default:
      printf("Invalid Direction!\n");
      return -1;

  }

  return dir;
}

int motor_set_speed(uint8_t dir, uint8_t speed){
  #if(DEBUG_FLAG)
  printf("motor_set_speed(%d, %d)\n", dir, speed);
  #endif

  switch(dir){
    case FORWARD:
      PCA9685_SetLevel(LEFT_FORWARD,   1);
      PCA9685_SetLevel(LEFT_REVERSE,   0);
      PCA9685_SetLevel(RIGHT_FORWARD,  1);
      PCA9685_SetLevel(RIGHT_REVERSE,  0);
      break;
    case REVERSE:
      PCA9685_SetLevel(LEFT_FORWARD,   0);
      PCA9685_SetLevel(LEFT_REVERSE,   1);
      PCA9685_SetLevel(RIGHT_FORWARD,  0);
      PCA9685_SetLevel(RIGHT_REVERSE,  1);
      break;

    default:
      printf("Invalid Direction!\n");
      return -1;

  }

  if(speed >= JOLT_SPEED){//If no jolt needed, return speed
    PCA9685_SetPwmDutyCycle(LEFT,  speed);
    PCA9685_SetPwmDutyCycle(RIGHT, speed);
    return speed;
  }
  else{//"Jolt" the motor by starting at a higher speed if the targeted speed is really low

    //This makes the time spent "jolting" consistent, entire process should be voer the course of a tenth of a second
    int delay_per = JOLT_DELAY / (JOLT_SPEED - speed);
    for(int i=JOLT_SPEED; i>speed; i--){
       PCA9685_SetPwmDutyCycle(LEFT,  i);
       PCA9685_SetPwmDutyCycle(RIGHT, i);
       usleep(delay_per);
    }

    return speed;
  }
}

int motor_stop(){
  #if(DEBUG_FLAG)
  printf("motor_stop()\n");
  #endif

  PCA9685_SetPwmDutyCycle(LEFT, 0);
  PCA9685_SetPwmDutyCycle(RIGHT, 0);

  return 0;
}

/* Turning */

int motor_pivot(uint8_t dir){
  #if(DEBUG_FLAG)
  printf("motor_pivot(%d)\n", dir);
  #endif

  switch(dir){
    case TURN_LEFT:
      PCA9685_SetLevel(LEFT_FORWARD,  0);
      PCA9685_SetLevel(LEFT_REVERSE,  1);
      PCA9685_SetLevel(RIGHT_FORWARD, 1);
      PCA9685_SetLevel(RIGHT_REVERSE, 0);
      break;
    case TURN_RIGHT:
      PCA9685_SetLevel(LEFT_FORWARD,  1);
      PCA9685_SetLevel(LEFT_REVERSE,  0);
      PCA9685_SetLevel(RIGHT_FORWARD, 0);
      PCA9685_SetLevel(RIGHT_REVERSE, 1);
      break;

    default:
      printf("[!] Invalid Direction!\n");
      return -1;
  }

  PCA9685_SetPwmDutyCycle(RIGHT, 100);
  PCA9685_SetPwmDutyCycle(RIGHT, 100);

  return 0;
}

int motor_steer(double heading, uint8_t speed, uint8_t dir){
  #if(DEBUG_FLAG)
  printf("motor_steer(%f, %d, $d)\n", heading, speed, dir);
  #endif

  switch(dir){
    case FORWARD:
      PCA9685_SetLevel(LEFT_FORWARD,   1);
      PCA9685_SetLevel(LEFT_REVERSE,   0);
      PCA9685_SetLevel(RIGHT_FORWARD,  1);
      PCA9685_SetLevel(RIGHT_REVERSE,  0);
      break;
    case REVERSE:
      PCA9685_SetLevel(LEFT_FORWARD,   0);
      PCA9685_SetLevel(LEFT_REVERSE,   1);
      PCA9685_SetLevel(RIGHT_FORWARD,  0);
      PCA9685_SetLevel(RIGHT_REVERSE,  1);
      break;

    default:
      printf("Invalid Direction!\n");
      return -1;

  }

  //Any input past the limits will be treated as it were at the limit
  if(heading  > 0.8)
    {return motor_pivot(TURN_RIGHT);}
  else if(heading < -0.8)
    {return motor_pivot(TURN_LEFT);}

  if(speed > 100){
    speed = 100;
  }
  else if(speed < 0){
    speed = 0;
  }

  int speed_left;
  int speed_right;
  
  
  if(speed >= JOLT_SPEED){//If no jolt needed
    if(heading <= 0.0){
      speed_left = (1 - (TURN_INTENSITY * abs(heading))) * speed;
      speed_right = speed;
    }
    else{
      speed_left = speed;
      speed_right = (1 - (TURN_INTENSITY * abs(heading))) * speed;
    }

    PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
    PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
    
    return speed;
  }
  else{//"Jolt" the motor by starting at a higher speed if the targeted speed is really low

    //This makes the time spent "jolting" consistent, entire process should be voer the course of a tenth of a second
    int delay_per = JOLT_DELAY / (JOLT_SPEED - speed);
    for(int i=JOLT_SPEED; i>speed; i--){

      if(heading <= 0.0){
        speed_left = (1 - (TURN_INTENSITY * abs(heading))) * i;
        speed_right = i;
      }
      else{
        speed_left = i;
        speed_right = (1 - (TURN_INTENSITY * abs(heading))) * i;
      }
      
      PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
      PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
      usleep(delay_per);
    }

    PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
    PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
    
    return speed;
  }

  return 0;
}

/* THREAD CONTROLLER FUNCTIONS */
void* th_motor_straight  (void* arg){

  #if(DEBUG_FLAG)
  printf("th_motor_straight(%p) START\n", arg);
  #endif

  //Extract information from passed struct
  motor_param_t* params = (motor_param_t*) arg;

  uint8_t direction = params->direction;
  uint8_t speed     = params->speed;
  clock_t time_limit = params->time_limit;


  //Begin Command

  if(motor_set_direction(direction) != direction){
    printf("[!] Failed to set motor direction!\n");
    return NULL;
  }

  if(speed >= JOLT_SPEED){//If no jolt needed, return speed
    PCA9685_SetPwmDutyCycle(LEFT,  speed);
    PCA9685_SetPwmDutyCycle(RIGHT, speed);
  }
  else{//"Jolt" the motor by starting at a higher speed if the targeted speed is really low

    //This makes the time spent "jolting" consistent, entire process should be over the course of a tenth of a second
    int delay_per = JOLT_DELAY / (JOLT_SPEED - speed);
    for(int i=JOLT_SPEED; i>speed; i--){
       PCA9685_SetPwmDutyCycle(LEFT,  i);
       PCA9685_SetPwmDutyCycle(RIGHT, i);
       usleep(delay_per);
    }
  }

  //Begin timer
  clock_t time_began = clock();

  while( (clock()-time_began) < time_limit){
    usleep(CLOCK_DELAY);
  }

  #if(DEBUG_FLAG)
  printf("th_motor_straight(%p) TERMINATE\n", arg);
  #endif

  return NULL;
}

void* th_motor_avoidpivot(void* arg){

  #if(DEBUG_FLAG)
  printf("th_motor_avoidpivot(%p) START\n", arg);
  #endif


  #if(DEBUG_FLAG)
  printf("th_motor_avoidpivot(%p) TERMINATE\n", arg);
  #endif

  return NULL;
}
void* th_motor_steering  (void* arg){

  #if(DEBUG_FLAG)
  printf("th_motor_steering(%p) START\n", arg);
  #endif

  //Extract information from passed struct
  motor_param_t* params = (motor_param_t*) arg;

  uint8_t direction  = params->direction;
  uint8_t speed      = params->speed;
  uint8_t heading    = params->heading;
  clock_t time_limit = params->time_limit;


  //Ensure Direction of motor is correct
  //Any input past the limits will be treated as it were at the limit
  if(heading  > 0.8)
    {direction = TURN_RIGHT;}
  else if(heading < -0.8)
    {direction = TURN_LEFT;}

  //Any speed in excess of 100 is 100, less than 0 is 0
  if(speed > 100){
    speed = 100;
  }
  else if(speed < 0){
    speed = 0;
  }

  if(motor_set_direction(direction) != direction){
    printf("[!] ERROR SETTING MOTOR DIRECTION!\n");
    return NULL;
  }

  //Begin Command
  clock_t time_began = clock();

  int speed_left;
  int speed_right;
  
  if(speed >= JOLT_SPEED){//If no jolt needed
    if(heading <= 0.0){
      speed_left = (1 - (TURN_INTENSITY * abs(heading))) * speed;
      speed_right = speed;
    }
    else{
      speed_left = speed;
      speed_right = (1 - (TURN_INTENSITY * abs(heading))) * speed;
    }

    PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
    PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
    
    return speed;
  }
  else{//"Jolt" the motor by starting at a higher speed if the targeted speed is really low

    //This makes the time spent "jolting" consistent, entire process should be voer the course of a tenth of a second
    int delay_per = JOLT_DELAY / (JOLT_SPEED - speed);
    for(int i=JOLT_SPEED; i>speed; i--){

      if(heading <= 0.0){
        speed_left = (1 - (TURN_INTENSITY * abs(heading))) * i;
        speed_right = i;
      }
      else{
        speed_left = i;
        speed_right = (1 - (TURN_INTENSITY * abs(heading))) * i;
      }
      
      PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
      PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
      usleep(delay_per);
    }

    PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
    PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
  }

  #if(DEBUG_FLAG)
  printf("th_motor_steering(%p) TERMINATE\n", arg);
  #endif

  return NULL;
}