// A Ghost module
#ifndef _GHOST_H_
#define _GHOST_H_
#include "map.h"
#include "types.h"

#define GHOST_NUM 4
// longer delay = slower ghost
#define GHOST_SPEED_DELAY 600
#define FRIGHTENED_DURATION_MS 10000
#define FRIGHTENED_GHOST_TRANSITION_DURATION_MS 3000

// Forward declare struct
// https://stackoverflow.com/questions/30185561/c-function-pointer-callback-as-struct-member-with-self-reference-parameter
typedef struct Ghost Ghost;

typedef void (*GhostCallback)(Ghost* ghost);
typedef void (*GhostColorChangeCallback)(Color newColor);
typedef enum
{
    CHASE=0,
    FRIGHTENED,
    PAUSED,
    UNKNOWN
} GhostMode;

typedef struct Ghost
{
    int id;
    char *name;
    GhostMode mode;
    long long modeStartTimeInMs;
    Location location;
    Tile currentTile;
    Direction currentDirection;
} Ghost;


void Ghost_init(Location ghostEntrance, Location otherGhosts[]);
void Ghost_cleanup();
void Ghost_registerCallback(GhostCallback newCallback, GhostColorChangeCallback newColorCallback);
Ghost *Ghost_getGhostAtLocation(Location location);
void Ghost_changeAllGhostMode(GhostMode newMode);
void Ghost_decreaseActiveGhostCount();

#endif