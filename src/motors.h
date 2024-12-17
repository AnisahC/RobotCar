




#define LEFT           PCA_CHANNEL_5
#define LEFT_FORWARD   PCA_CHANNEL_3
#define LEFT_REVERSE   PCA_CHANNEL_4
#define RIGHT          PCA_CHANNEL_0
#define RIGHT_FORWARD  PCA_CHANNEL_2
#define RIGHT_REVERSE  PCA_CHANNEL_1

#define FORWARD 1
#define REVERSE 0

#define TURN_LEFT  0
#define TURN_RIGHT 1


/* Basic Controls */
int motor_set_speed(uint8_t dir, uint8_t speed);
int motor_stop();


/* Turning */
//This is expected to be a strict 0-degree turning radius
int motor_pivot(uint8_t dir);
//This is expect to adjust the path forward slightly to the side
//Pass in a double from -1 to 1:
// - Closer to -1 means the sharper the turn left
// - Closer to +1 means the sharper the turn right
int motor_steer(double dir, int speed, int heading);
