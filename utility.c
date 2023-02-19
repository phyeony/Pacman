#include "utility.h"
#include <time.h>

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