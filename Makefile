DIR_LIB = lib
DIR_BIN = bin
DIR_Config = lib/Config
DIR_PCA9685 = lib/PCA9685
DIR_TESTING = testing
DIR_MAIN = ./
DIR_SRC = src

OBJ_C = $(wildcard ${DIR_LIB}/*.c ${DIR_SRC}/*.c ${DIR_Config}/*.c ${DIR_PCA9685}/*.c)
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = motor_control
#BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc

DEBUG = -g -O0 -Wall
CFLAGS += $(DEBUG)

# USELIB = USE_BCM2835_LIB
# USELIB = USE_WIRINGPI_LIB
USELIB = USE_DEV_LIB
DEBUG = -D $(USELIB) 
ifeq ($(USELIB), USE_BCM2835_LIB)
    LIB = -lbcm2835 -lm 
else ifeq ($(USELIB), USE_WIRINGPI_LIB)
    LIB = -lwiringPi -lm 

endif



${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIB) -lm -lpigpio -lrt -lpthread

${DIR_BIN}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -lpigpio -lrt -lpthread -I $(DIR_LIB) -I $(DIR_Config)  -I $(DIR_PCA9685)

bin/main.o : src/main.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -lpigpio -lrt -lpthread -I $(DIR_LIB) -I $(DIR_Config)  -I $(DIR_PCA9685)

bin/sensors.o : src/sensors.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -lpigpio -lrt -lpthread -I $(DIR_LIB) -I $(DIR_Config)  -I $(DIR_PCA9685)

${DIR_BIN}/%.o : $(DIR_LIB)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_Config) -I $(DIR_OBJ) -I $(DIR_PCA9685)

${DIR_BIN}/%.o : $(DIR_Config)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_Config)

${DIR_BIN}/%.o : $(DIR_PCA9685)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) -I $(DIR_Config)

build:
	gcc -D USE_DEV_LIB  -c  lib/Config/DEV_Config.c -o bin/DEV_Config.o  -I lib/Config
	gcc -D USE_DEV_LIB  -c  lib/Config/dev_hardware_SPI.c -o bin/dev_hardware_SPI.o  -I lib/Config
	gcc -D USE_DEV_LIB  -c  lib/Config/dev_hardware_i2c.c -o bin/dev_hardware_i2c.o  -I lib/Config
	gcc -D USE_DEV_LIB  -c  lib/Config/sysfs_gpio.c -o bin/sysfs_gpio.o  -I lib/Config
	gcc -D USE_DEV_LIB  -c  lib/PCA9685/PCA9685.c -o bin/PCA9685.o  -I lib/Config
	gcc -D USE_DEV_LIB  -c  src/sensors.c -o bin/sensors.o
	gcc -D USE_DEV_LIB  -c  src/main.c -o bin/main.o -lpthread -lpigpio -lrt -I lib/Config -I lib/PCA9685/
	gcc -D USE_DEV_LIB      bin/main.o bin/sensors.o bin/DEV_Config.o bin/dev_hardware_SPI.o bin/dev_hardware_i2c.o bin/sysfs_gpio.o bin/PCA9685.o -o motor_control  -lm -lpigpio -lrt -lpthread

clean:
	rm $(DIR_BIN)/*.* 
	rm $(TARGET) 

run:
	sudo ./motor_control
