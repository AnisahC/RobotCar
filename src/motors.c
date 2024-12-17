

#include <stdlib.h>
#include "PCA9685.h"
#include "motors.h"

#define DEBUG_FLAG 1
#define JOLT_SPEED 50
#define JOLT_DELAY 100000
#define TURN_INTENSITY 1.0

/* Basic Controls */
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

int motor_steer(double dir, int speed, int heading){
  #if(DEBUG_FLAG)
  printf("motor_steer(%f)\n", dir);
  #endif

  switch(heading){
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
  if(dir > 1.0)
    {dir = 1.0;}
  else if(dir < -1.0)
    {dir = -1.0;}

  if(speed > 100){
    speed = 100;
  }
  else if(speed < 0){
    speed = 0;
  }

  int speed_left;
  int speed_right;
  
  
  if(speed >= JOLT_SPEED){//If no jolt needed
    if(dir <= 0.0){
      speed_left = (1 - (TURN_INTENSITY * abs(dir))) * speed;
      speed_right = speed;
    }
    else{
      speed_left = speed;
      speed_right = (1 - (TURN_INTENSITY * abs(dir))) * speed;
    }

    PCA9685_SetPwmDutyCycle(LEFT,  speed_left);
    PCA9685_SetPwmDutyCycle(RIGHT, speed_right);
    
    return speed;
  }
  else{//"Jolt" the motor by starting at a higher speed if the targeted speed is really low

    //This makes the time spent "jolting" consistent, entire process should be voer the course of a tenth of a second
    int delay_per = JOLT_DELAY / (JOLT_SPEED - speed);
    for(int i=JOLT_SPEED; i>speed; i--){

      if(dir <= 0.0){
        speed_left = (1 - (TURN_INTENSITY * abs(dir))) * i;
        speed_right = i;
      }
      else{
        speed_left = i;
        speed_right = (1 - (TURN_INTENSITY * abs(dir))) * i;
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
