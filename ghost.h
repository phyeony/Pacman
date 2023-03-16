// A Ghost module
#ifndef _GHOST_H_
#define _GHOST_H_
#include "map.h"

#define GHOST_NUM 4
#define FRIGHTENED_DURATION_MS 5000

//Forward declare struct
// https://stackoverflow.com/questions/30185561/c-function-pointer-callback-as-struct-member-with-self-reference-parameter
typedef struct Ghost Ghost;

typedef void (*GhostCallback)(Ghost *ghost);
typedef enum
{
    CHASE=0,
    FRIGHTENED,
    PAUSED,
    UNKNOWN
} GhostMode;

typedef struct Ghost
{
    char *name;
    GhostMode mode;
    long long modeStartTimeInMs;
    GhostCallback movementCallback;
    Location location;
} Ghost;


void Ghost_init();
void Ghost_cleanup();
// void Ghost_registerCallback(Ghost ghosts[], int size, GhostCallback callback);

#endif