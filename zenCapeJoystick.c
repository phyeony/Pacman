#include "zenCapeJoystick.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "utility.h"

#define MAX_REQUEST_NUM 20
#define VOLUME_DEBOUNCE_MS 300
#define TEMPO_DEBOUNCE_MS 300
#define MODE_DEBOUNCE_MS 300
#define GPIO_PIN_TYPE "gpio"

// Local headers
static void *startReadingInputs(void *args);
static void moveUp(void);
static void moveDown(void);
static void moveLeft(void);
static void moveRight(void);

static int exitJoystick = 0;
static JoystickCallback cb;
static pthread_t id;

typedef struct
{
    char *directionName;
    int linuxGpioNum;
    int pinHeaderNum;
    int pinNum;
    long long debounceTimeInMs;
    void (*action)(void);
} Joystick;

static Joystick joysticks[] = {
    [UP]={"up", .linuxGpioNum = 26, .pinHeaderNum=8, .pinNum=14, .debounceTimeInMs=MODE_DEBOUNCE_MS, .action=&moveUp},
    [DOWN]={"down", .linuxGpioNum = 46, .pinHeaderNum=8, .pinNum=16, .debounceTimeInMs=VOLUME_DEBOUNCE_MS, .action=&moveDown},
    [LEFT]={"left", .linuxGpioNum = 65, .pinHeaderNum=8, .pinNum=18, .debounceTimeInMs=VOLUME_DEBOUNCE_MS, .action=&moveLeft},
    [RIGHT]={"right", .linuxGpioNum = 47, .pinHeaderNum=8, .pinNum=15, .debounceTimeInMs=TEMPO_DEBOUNCE_MS, .action=&moveRight},
    // [PUSHED]={"pushed", .linuxGpioNum = 27, .pinHeaderNum=8, .pinNum=17, .debounceTimeInMs=TEMPO_DEBOUNCE_MS, .action=NULL},
    [IDLE_STATE]={"idle",0,0,0,0,NULL}
};

static void moveUp(void)
{
    cb(UP);
}
static void moveDown(void)
{
    cb(DOWN);
}
static void moveLeft(void)
{
    cb(LEFT);
}
static void moveRight(void)
{
    cb(RIGHT);
}

static void *startReadingInputs(void *args) {
    Joystick currentJoystick;
    long long lastActionTime=0;
    while(exitJoystick!=1){
        currentJoystick = joysticks[IDLE_STATE];

        // Read inputs from joystick. 
        // Note that we are not iterating over IDLE_STATE.
        for(int i=0; i<IDLE_STATE; i++) {
            if(Utility_readGPIOValueFile(joysticks[i].linuxGpioNum) == 0){
                currentJoystick = joysticks[i];
                break;
            }
        }
        long long currentTime = Utility_getCurrentTimeInMs();
        // Ignore actions that has been triggered before the debounce time passed
        if(currentJoystick.action != joysticks[IDLE_STATE].action) {
            if(currentTime - lastActionTime >= currentJoystick.debounceTimeInMs) {
                currentJoystick.action();
                lastActionTime = currentTime;
                printf("Action: %s \n", currentJoystick.directionName);
            } 
            //else {
                //printf("Debounced action: %s \n", currentJoystick.directionName);
            //}
        }

        Utility_sleepForMs(10);
    }
    return NULL;
}


// this function is to be called by gameManager to pass in a function pointer that moves pacman
void ZenCapeJoystick_registerCallback(JoystickCallback onJoyconInput)
{
    cb = onJoyconInput;
}

void ZenCapeJoystick_init()
{
    printf("Start Zen Cape Joystick...\n");
    // set up pins
    for(int i = 0; i <IDLE_STATE; i++){
        //config pin as GPIO
        Utility_configPin(joysticks[i].pinHeaderNum, joysticks[i].pinNum, GPIO_PIN_TYPE);
        Utility_exportGpioIfNeeded(joysticks[i].linuxGpioNum);
    }

    if (pthread_create(&id, NULL, &startReadingInputs, NULL) != 0)
    {
        perror("Failed to create a new thread...");
    }
}

void ZenCapeJoystick_cleanup()
{
    printf("Cleanup Zen Cape Joystick...\n");
    
    exitJoystick = 1;

    pthread_join(id, NULL);
}