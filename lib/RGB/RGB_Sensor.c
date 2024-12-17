/**************************************************************
* Class:: CSC-615-01 Fall 2024
* Team Name:: Fire Hawks
* Names:: Angelo Arriaga, Anisah Chowdhury, Austin Ng, Citlalin Galvan,
            Owen Meyer
* Github-Name:: AnisahC
* Project:: RBG Sensor Library
*
* File:: RGB_Sensor.c
*
* Description:: A library/module that can be easily included in another project’s
                main program using TCS34725 sensor. It will call the necessary
                functions and output will be BOTH an R G B Value set (in Hex) and
                a Color Name (Color can be approximated with a confidence value)
*
**************************************************************/

#include <stdio.h> 
#include <pigpio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RGB_Sensor.h"

// I2C handle
int tcs34725_handle;
RGB_Val* val;

// Predefined colors
RGB_Val colors[] = {
  {255, 255, 0},  // Yellow
  {250, 130, 20},  // Orange
  {255, 0, 0},    // Red
  {128, 0, 128},  // Purple
  {0, 0, 255},    // Blue
  {25, 178, 25},    // Green
  {0, 0, 0},      // Black
  {255, 255, 255} // White
};

// Color names corresponding to predefined colors
char* color_names[] = {
  "Yellow",
  "Orange",
  "Red",
  "Purple",
  "Blue",
  "Green",
  "Black",
  "White"
};

// Function to write a byte to the sensor
int write_byte(uint8_t reg, uint8_t value) {
    return i2cWriteByteData(tcs34725_handle, TCS34725_COMMAND_BIT | reg, value);
}

// Initialize the TCS34725 chip
int TCS34725_init(){
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialisation failed\n");
        return -1;
    }

    tcs34725_handle = i2cOpen(I2C_BUS, TCS34725_ADDRESS, 0);

    if (tcs34725_handle < 0) {
        fprintf(stderr, "i2cOpen failed\n");
        return -1;
    }

    // Enable the device by setting the PON bit
    int a = i2cWriteByteData(tcs34725_handle, TCS34725_COMMAND_BIT | TCS34725_ENABLE,
                            TCS34725_ENABLE_PON);
    if(a < 0){
        printf("details: %d\n",a);
        fprintf(stderr, "enable failed\n");
        return -1;
    }

    // Enables the ADC, for measuring light
    // activates the internal oscillator (required for the sensor to work) and the color sending
    if (i2cWriteByteData(tcs34725_handle, TCS34725_COMMAND_BIT | TCS34725_ENABLE,
        TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN) < 0) {
        fprintf(stderr, "measuring failed\n");
        return -1;
    }

    // Set the integration time
    // determines how long the sensor collects light for
    // longer time = more light collected
    if (i2cWriteByteData(tcs34725_handle, TCS34725_COMMAND_BIT | TCS34725_ATIME,
        TCS34725_INTEGRATIONTIME_101MS) < 0){
        fprintf(stderr, "integration time failed\n");
        return -1;
    }

    // Set the gain level: amplifies sensor's response to light
    // Higher gain values will make the sensor more sensitive to light
    if(i2cWriteByteData(tcs34725_handle, TCS34725_COMMAND_BIT | TCS34725_CONTROL,
        TCS34725_GAIN_16X) < 0) {
        fprintf(stderr, "gain level failed\n");
        return -1;
    }

    gpioDelay(2000);

    val = (RGB_Val*)malloc(sizeof(RGB_Val));

    if(val == NULL){
        return -1;
    }

    // printf("Sensor initialized\n");
    return 0;

}

// Read a word from the I2C connection
uint16_t readWordi2c(int handle, uint8_t reg_addr) {
    // Read low byte
    int low = i2cReadByteData(handle, reg_addr | TCS34725_COMMAND_BIT);
    if (low < 0) {
        fprintf(stderr, "Failed to read low byte\n");
        return 0;
    }

    // Read high byte 
    int high = i2cReadByteData(handle, (reg_addr + 1) | TCS34725_COMMAND_BIT);
    if (high < 0) {
        fprintf(stderr, "Failed to read high byte\n");
        return 0;
    }

    // Combine high and low bytes
    return (uint16_t)((high << 8) | low);
}

// Read the RGB values from the TCS34275 into the global values
int readRGB(int handle) {
    uint16_t red = 0, green = 0, blue = 0;
    uint16_t red_sum = 0, green_sum = 0, blue_sum = 0;
    int num_reads = 5;

    // Read RGB values 10 times and accumulate the results
    for (int i = 0; i < num_reads; i++) {
        red = readWordi2c(handle, TCS34725_RDATAL);
        green = readWordi2c(handle, TCS34725_GDATAL);
        blue = readWordi2c(handle, TCS34725_BDATAL);

        // Accumulate the values
        red_sum += (red >> 8);
        green_sum += (green >> 8);
        blue_sum += (blue >> 8);

        usleep(10000);
    }

    // Check if any of the readings are invalid 
    if (red_sum < 0 || green_sum < 0 || blue_sum < 0) {
        return -1; 
    }

    // Calculate average
    val->red = (uint8_t)((red_sum / num_reads)); 
    val->green = (uint8_t)((green_sum / num_reads));
    val->blue = (uint8_t)((blue_sum / num_reads));

    return 0;
}


// Euclidean distance for unsigned integers
float colorDistance(RGB_Val* color1, RGB_Val* color2) {
    // Ensure no underflow occurs by subtracting the smaller value from the larger
    int red_distance = (color1->red > color2->red) ? 
        (color1->red - color2->red) : (color2->red - color1->red);
    int green_distance = (color1->green > color2->green) ? 
        (color1->green - color2->green) : (color2->green - color1->green);
    int blue_distance = (color1->blue > color2->blue) ? 
        (color1->blue - color2->blue) : (color2->blue - color1->blue);

    return sqrtf(red_distance * red_distance + green_distance *
                 green_distance + blue_distance * blue_distance);
}


char* getColorName(RGB_Val* val) {
    float min_distance = 1000;
    int color_index = 0;

    for(int i=0; i<sizeof(colors)/sizeof(colors[0]); i++){
        float distance = colorDistance(val, &colors[i]);
        //printf("Distance with %s: %f\n", color_names[i], distance);
        if(distance < min_distance){
            min_distance = distance;
            color_index = i;
        }
    }

    return color_names[color_index];
}

float getConfidence(RGB_Val* rgb){
    float min_distance = 1000;

    for(int i=0; i<sizeof(colors)/sizeof(colors[0]); i++){
        float distance = colorDistance(val, &colors[i]);
        if(distance < min_distance){
            min_distance = distance;
        }
    }

    // find the max distance (black and white)
    float max_distance = sqrt(255*255 + 255*255 + 255*255);
    float confidence = 1.0 - (min_distance / max_distance);

    return confidence;
}

// Cleanup function to close I2C
void TCS34725_Close() {
    i2cClose(tcs34725_handle);
    free(val);
    val = NULL;
}

void getRGB(RGB_Val* userStruct){
    if(readRGB(tcs34725_handle) < 0 ) {
        printf("Failed to read RGB value\n");
    } else {
        memcpy(userStruct,val,sizeof(RGB_Val));
    }
}
