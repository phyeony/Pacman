#include "ghost.h"
#include "utility.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static Ghost ghosts[GHOST_NUM]={
    {.name = "ghostA", .mode = PAUSED,},
    {.name = "ghostB", .mode = PAUSED,},
    {.name = "ghostC", .mode = PAUSED,},
    {.name = "ghostD", .mode = PAUSED,},
};
static int running = 1;
static pthread_t id;
void* startMovingGhosts();
static GhostCallback movementCallback;

void Ghost_registerCallback(GhostCallback newCallback)
{
    movementCallback = newCallback;
}

void* startMovingGhosts()
{
    int headStart = 9;
    int activeGhosts = 0;
    
    while (running) {

        // logic for chasing only, have to change when implement frightened
        
        for (int i = 0; i < activeGhosts; i++){
            (*movementCallback)(&ghosts[i]);
            Utility_sleepForMs(GHOST_SPEED_DELAY);
        }
        if (activeGhosts < 4){
            headStart++;
        }
        if (headStart == 10){
            headStart = 0;
            ghosts[activeGhosts].mode = CHASE;
            activeGhosts++;
        }
    }
    return NULL;
}

// game manager will call this and pass in the ghost entrance and locations of all ghosts
// i think game manager should start ghost thread because the existence of ghosts depends on the existence of a map?
void Ghost_init(Location ghostEntrance, Location otherGhosts[])
{
    ghosts[0].location = ghostEntrance;
    ghosts[0].currentTile = empty;
    for (int i = 1; i < GHOST_NUM; i++){
        ghosts[i].location = otherGhosts[i - 1];
        ghosts[i].currentTile = empty;
    }
    if (pthread_create(&id, NULL, &startMovingGhosts, NULL) != 0)
    {
        perror("Failed to create a new thread...");
    }
}

void Ghost_cleanup()
{
    running = 0;
    pthread_join(id, NULL);
}
