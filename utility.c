#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define GPIO_FILE_PATH_SIZE 40
#define GPIO_PATH "/sys/class/gpio"
#define CONFIG_PIN_COMMAND "config-pin p%d.%d %s"
#define COMMAND_SIZE 30

// Local headers
static void runCommand(char *);

static void runCommand(char *command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0)
    {
        perror("Unable to execute command:");
        printf(" command: %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }
}

// wait a number of milliseconds
void Utility_sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;

    long long delayNS = delayInMs * NS_PER_MS;
    int seconds = delayNS / NS_PER_SECOND;
    int nanoseconds = delayNS % NS_PER_SECOND;

    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

long long Utility_getCurrentTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

void Utility_exportGpioIfNeeded(int gpioNum)
{
    char filePath[GPIO_FILE_PATH_SIZE];
    snprintf(filePath, GPIO_FILE_PATH_SIZE, GPIO_PATH "/gpio%d", gpioNum);
    if (access(filePath, F_OK) == -1)
    {
        // If the gpio file does not exist, export gpio
        FILE *gpioExP = fopen(GPIO_PATH "export", "w");
        if (gpioExP == NULL)
        {
            printf("ERROR: Unable to open export file.\n");
            exit(-1);
        }
        fprintf(gpioExP, "%d", gpioNum);
        fclose(gpioExP);
    }
    Utility_sleepForMs(100);
}

int Utility_readGPIOValueFile(int gpioNum)
{
    FILE *file;
    int val;
    char filePath[GPIO_FILE_PATH_SIZE];
    snprintf(filePath, GPIO_FILE_PATH_SIZE, GPIO_PATH "/gpio%d/value", gpioNum);
    file = fopen(filePath, "r");
    if (file == NULL)
    {
        printf("ERROR OPENING %s.", filePath);
        perror("");
        exit(-1);
    }
    fscanf(file, "%d", &val);
    // Closing the file
    fclose(file);
    return val;
}

void Utility_configPin(int pinHeaderNum, int pinNum, char *type)
{
    char command[COMMAND_SIZE];
    snprintf(command, COMMAND_SIZE, CONFIG_PIN_COMMAND, pinHeaderNum, pinNum, type);
    runCommand(command);
}
