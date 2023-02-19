/********************************************************************
 *  File Name: ledDisplay.c
 *  Description: A simple program to display pattern on LED Matrix
 *  
 *  About 80% of the code converted from Python to C, source:
 *      https://learn.adafruit.com/connecting-a-16x32-rgb-led-matrix-panel-to-a-raspberry-pi/experimental-python-code
 *-------------------------------------------------------------------
 *  Created by: Janet mardjuki
 *  Date: 3 December 2015
 *  
 *  Modified by: Raymond Chan
 *  Date: 2 August 2018
 * 
 *  Modified by: Pacman team
 *  Date: 19 Feb 2023
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "utility.h"
#include "gameManager.h"

/*** GLOBAL VARIABLE ***/
/* GPIO PATH */
#define GPIO_PATH "/sys/class/gpio/"

/* GPIO Pins Definition */
#define RED1_PIN 78     // UPPER
#define GREEN1_PIN 70
#define BLUE1_PIN 8
#define RED2_PIN 71 // LOWER
#define GREEN2_PIN 72
#define BLUE2_PIN 73
#define CLK_PIN 79      // Arrival of each data
#define LATCH_PIN 76    // End of a row of data
#define OE_PIN 80       // Transition from one row to another
#define A_PIN 75        // Row select
#define B_PIN 74
#define C_PIN 77

#define FILE_PATH_SIZE 70

#define S_IWRITE "S_IWUSR"

/* TIMING */
#define DELAY_IN_US 5
#define DELAY_IN_SEC 0

static pthread_t id;

/* FILES HANDLER */
static int fileDesc_red1;
static int fileDesc_blue1;
static int fileDesc_green1;
static int fileDesc_red2;
static int fileDesc_blue2;
static int fileDesc_green2;
static int fileDesc_clk;
static int fileDesc_latch;
static int fileDesc_oe;
static int fileDesc_a;
static int fileDesc_b;
static int fileDesc_c;


/**
 * exportAndOut
 * Export a pin and set the direction to output
 * @params
 *  int pinNum: the pin number to be exported and set for output
 */
static void exportAndOut(int pinNum)
{
    // Change the direction into out
    char fileNameBuffer[1024];
    sprintf(fileNameBuffer, GPIO_PATH "gpio%d/direction", pinNum);
        
    FILE *gpioDirP = fopen(fileNameBuffer, "w");
    if (gpioDirP == NULL) {
        perror("");
        printf("Trying to export pin #%d...\n", pinNum);
        // Export the gpio pins
        FILE *gpioExP = fopen(GPIO_PATH "export", "w");
        if ( gpioExP == NULL ){
            printf("ERROR: Unable to open export file.\n");
            exit(-1);
        }
        fprintf(gpioExP, "%d", pinNum);
        fclose(gpioExP);
        Utility_sleepForMs(1000);
        gpioDirP = fopen(fileNameBuffer, "w");
    }
    if(gpioDirP == NULL) {
        printf("Unable to write direction in pin #%d",pinNum);
        perror("");
        exit(-1);
    }
    fprintf(gpioDirP, "out");
    fclose(gpioDirP);

    return;
}

static int getFileDescriptor(int pinNum) {
    char fileNameBuffer[FILE_PATH_SIZE];
    snprintf(fileNameBuffer,FILE_PATH_SIZE, GPIO_PATH "gpio%d/value", pinNum);
    return open(fileNameBuffer, O_WRONLY, S_IWRITE);
}

/**
 * ledDisplay_setupPins
 * Setup the pins used by the led matrix, by exporting and set the direction to out
 */
static void ledDisplay_setupPins(void)
{   
    // !Upper led
    exportAndOut(RED1_PIN);
    fileDesc_red1 = getFileDescriptor(RED1_PIN);
    exportAndOut(GREEN1_PIN);
    fileDesc_green1 = getFileDescriptor(GREEN1_PIN);
    exportAndOut(BLUE1_PIN);
    fileDesc_blue1 = getFileDescriptor(BLUE1_PIN);

    // Lower led
    exportAndOut(RED2_PIN);
    fileDesc_red2 = getFileDescriptor(RED2_PIN);
    exportAndOut(GREEN2_PIN);
    fileDesc_green2 = getFileDescriptor(GREEN2_PIN);
    exportAndOut(BLUE2_PIN);
    fileDesc_blue2 = getFileDescriptor(BLUE2_PIN);

    // Timing
    exportAndOut(CLK_PIN);
    fileDesc_clk = getFileDescriptor(CLK_PIN);
    exportAndOut(LATCH_PIN);
    fileDesc_latch = getFileDescriptor(LATCH_PIN);
    exportAndOut(OE_PIN);
    fileDesc_oe = getFileDescriptor(OE_PIN);

    // Row Select
    exportAndOut(A_PIN);
    fileDesc_a = getFileDescriptor(A_PIN);
    exportAndOut(B_PIN);
    fileDesc_b = getFileDescriptor(B_PIN);
    exportAndOut(C_PIN);
    fileDesc_c = getFileDescriptor(C_PIN); 

    return;
}

/** 
 *  ledDisplay_clock
 *  Generate the clock pins
 */
static void ledDisplay_clock(void)
{
    // Bit-bang the clock gpio
    // Notes: Before program writes, must make sure it's on the 0 index
    lseek(fileDesc_clk, 0, SEEK_SET);
    write(fileDesc_clk, "1", 1);
    lseek(fileDesc_clk, 0, SEEK_SET);
    write(fileDesc_clk, "0", 1);

    return;
}

/**
 *  ledDisplay_latch
 *  Generate the latch pins
 */
static void ledDisplay_latch(void)
{
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "1", 1);
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "0", 1);

    return;
}

/**
 *  ledDisplay_bitsFromInt
 *  Convert integer passed on into bits and put in array
 *  @params:
 *      int * arr: pointer to array to be filled with bits
 *      int input: integer to be converted to bits
 */
static void ledDisplay_bitsFromInt(int * arr, int input)
{
    arr[0] = input & 1;

    arr[1] = input & 2;
    arr[1] = arr[1] >> 1;

    arr[2] = input & 4;
    arr[2] = arr[2] >> 2;

    return;
}

/**
 *  ledDisplay_setRow
 *  Set LED Matrix row
 *  @params:
 *      int rowNum: the rowNumber to be inputted to row pins
 */
static void ledDisplay_setRow(int rowNum)
{
    // Convert rowNum single bits from int
    int arr[3] = {0, 0, 0};
    ledDisplay_bitsFromInt(arr, rowNum);

    // Write on the row pins
    char a_val[2];
    sprintf(a_val, "%d", arr[0]);
    lseek(fileDesc_a, 0, SEEK_SET);
    write(fileDesc_a, a_val, 1);

    char b_val[2];
    sprintf(b_val, "%d", arr[1]);
    lseek(fileDesc_b, 0, SEEK_SET);
    write(fileDesc_b, b_val, 1);

    char c_val[2];
    sprintf(c_val, "%d", arr[2]);
    lseek(fileDesc_c, 0, SEEK_SET);
    write(fileDesc_c, c_val, 1);


    return;
}

/**
 *  ledDisplay_setColourTop
 *  Set the colour of the top part of the LED
 *  @params:
 *      int colour: colour to be set
 */
static void ledDisplay_setColourTop(int colour)
{
    int arr[3] = {0, 0, 0};
    ledDisplay_bitsFromInt(arr, colour);

    // Write on the colour pins
    char red1_val[2];
    sprintf(red1_val, "%d", arr[0]);
    lseek(fileDesc_red1, 0, SEEK_SET);
    write(fileDesc_red1, red1_val, 1);

    char green1_val[2];
    sprintf(green1_val, "%d", arr[1]);
    lseek(fileDesc_green1, 0, SEEK_SET);
    write(fileDesc_green1, green1_val, 1);

    char blue1_val[2];
    sprintf(blue1_val, "%d", arr[2]);
    lseek(fileDesc_blue1, 0, SEEK_SET);
    write(fileDesc_blue1, blue1_val, 1);    

    return;
}

/**
 *  ledDisplay_setColourBottom
 *  Set the colour of the bottom part of the LED
 *  @params:
 *      int colour: colour to be set
 */
static void ledDisplay_setColourBottom(int colour)
{
    int arr[3] = {0,0,0};
    ledDisplay_bitsFromInt(arr, colour);

    // Write on the colour pins
    char red2_val[2];
    sprintf(red2_val, "%d", arr[0]);
    lseek(fileDesc_red2, 0, SEEK_SET);
    write(fileDesc_red2, red2_val, 1);

    char green2_val[2];
    sprintf(green2_val, "%d", arr[1]);
    lseek(fileDesc_green2, 0, SEEK_SET);
    write(fileDesc_green2, green2_val, 1);

    char blue2_val[2];
    sprintf(blue2_val, "%d", arr[2]);
    lseek(fileDesc_blue2, 0, SEEK_SET);
    write(fileDesc_blue2, blue2_val, 1);      
    
    // printf("[");
    // for(int i=0; i< 3; i++) {
    //     printf("%d ,",arr[i]);
    // }
    // printf("]\n");
    return;
}
/**
 *  ledDisplay_refresh
 *  Fill the LED Matrix with the respective pixel colour
 */
static void ledDisplay_refresh(void)
{
    Tile gameMap[ROW_SIZE][COLUMN_SIZE];
    for ( int rowNum = 0; rowNum < ROW_SIZE/2; rowNum++ ) {
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "1", 1); 
        ledDisplay_setRow(rowNum);
        GameManager_getMap(gameMap);
        for ( int colNum = 0; colNum < COLUMN_SIZE;  colNum++) {
            ledDisplay_setColourTop(gameMap[rowNum][colNum].color);
            ledDisplay_setColourBottom(gameMap[rowNum+8][colNum].color);
            ledDisplay_clock();
        }
        ledDisplay_latch();
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "0", 1); 
        struct timespec reqDelay = {DELAY_IN_SEC, DELAY_IN_US}; // sleep for delay
    	nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    return;
}

static void *startDisplaying(void *args)
{   
    while(1) {
        ledDisplay_refresh();
    }
    return NULL;
}

void LedDisplay_init(void)
{
    printf("Init ledDisplay...\n");
    // Setup pins
    ledDisplay_setupPins();

    if (pthread_create(&id, NULL, &startDisplaying, NULL) != 0)
    {
        perror("Failed to create a new thread...");
    }

}


void LedDisplay_cleanup(void)
{
    pthread_join(id, NULL);
}

