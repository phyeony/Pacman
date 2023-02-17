/********************************************************************
 *  File Name: test_ledMatrix.c
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
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

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

/* LED Screen Values */
static int screen[32][16];

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



// wait a number of milliseconds
void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;

    long long delayNS = delayInMs * NS_PER_MS;
    int seconds = delayNS / NS_PER_SECOND;
    int nanoseconds = delayNS % NS_PER_SECOND;

    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

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
        sleepForMs(1000);
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
 * ledMatrix_setupPins
 * Setup the pins used by the led matrix, by exporting and set the direction to out
 */
static void ledMatrix_setupPins(void)
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
 *  ledMatrix_clock
 *  Generate the clock pins
 */
static void ledMatrix_clock(void)
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
 *  ledMatrix_latch
 *  Generate the latch pins
 */
static void ledMatrix_latch(void)
{
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "1", 1);
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "0", 1);

    return;
}

/**
 *  ledMatrix_bitsFromInt
 *  Convert integer passed on into bits and put in array
 *  @params:
 *      int * arr: pointer to array to be filled with bits
 *      int input: integer to be converted to bits
 */
static void ledMatrix_bitsFromInt(int * arr, int input)
{
    arr[0] = input & 1;

    arr[1] = input & 2;
    arr[1] = arr[1] >> 1;

    arr[2] = input & 4;
    arr[2] = arr[2] >> 2;

    return;
}

/**
 *  ledMatrix_setRow
 *  Set LED Matrix row
 *  @params:
 *      int rowNum: the rowNumber to be inputted to row pins
 */
static void ledMatrix_setRow(int rowNum)
{
    // Convert rowNum single bits from int
    int arr[3] = {0, 0, 0};
    ledMatrix_bitsFromInt(arr, rowNum);

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
 *  ledMatrix_setColourTop
 *  Set the colour of the top part of the LED
 *  @params:
 *      int colour: colour to be set
 */
static void ledMatrix_setColourTop(int colour)
{
    int arr[3] = {0, 0, 0};
    ledMatrix_bitsFromInt(arr, colour);

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
 *  ledMatrix_setColourBottom
 *  Set the colour of the bottom part of the LED
 *  @params:
 *      int colour: colour to be set
 */
static void ledMatrix_setColourBottom(int colour)
{
    int arr[3] = {0,0,0};
    ledMatrix_bitsFromInt(arr, colour);

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
 *  ledMatrix_refresh
 *  Fill the LED Matrix with the respective pixel colour
 */
static void ledMatrix_refresh(void)
{
    for ( int rowNum = 0; rowNum < 8; rowNum++ ) {
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "1", 1); 
        ledMatrix_setRow(rowNum);
        for ( int colNum = 0; colNum < 32;  colNum++) {
            ledMatrix_setColourTop(screen[colNum][rowNum]);
            ledMatrix_setColourBottom(screen[colNum][rowNum+8]);
            ledMatrix_clock();
        }
        ledMatrix_latch();
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "0", 1); 
        struct timespec reqDelay = {DELAY_IN_SEC, DELAY_IN_US}; // sleep for delay
    	nanosleep(&reqDelay, (struct timespec *) NULL);
        // sleepForMs(1000);
    }
    return;
}

/**
 *  ledMatrix_setPixel
 *  Set the pixel selected on LED MAtrix with the colour selected
 *  @params:
 *      int x: x-axis
 *      int y: y-axis
 *      int colour: colour selected
 */
static void ledMatrix_setPixel(int x, int y, int colour)
{
    screen[y][x] = colour;

    return;
}

/*** MAIN ***/
int main()
{   
    // Reset the screen
    memset(screen, 0, sizeof(screen));

    // Setup pins
    ledMatrix_setupPins();
   
    for ( int i = 0; i < 16; i++ ) {
        ledMatrix_setPixel(i, i, 1);
        ledMatrix_setPixel(i, 32-1-i , 2);
   }

    printf("Starting the program\n");
    while(1) {
        ledMatrix_refresh();
    }
 
    return 0;
}

