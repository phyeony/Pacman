#include "ghost.h"
#include "utility.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static Ghost ghosts[GHOST_NUM]={
    {.name = "ghostA", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.name = "ghostB", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.name = "ghostC", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.name = "ghostD", .mode = PAUSED, .currentDirection = IDLE_STATE},
};
static int activeGhosts = 0;
static int running = 1;
static int headStart = 9;

static pthread_t id;

void* startMovingGhosts();
static GhostCallback movementCallback;

// Ghost *getGhostAtLocation(int row, int col) {
//     for (int i = 0; i < GHOST_NUM; i++) {
//         if (ghosts[i].location.row == row && ghosts[i].location.col == col) {
//             return &ghosts[i];
//         }
//     }
//     return NULL;
// }

void Ghost_registerCallback(GhostCallback newCallback)
{
    movementCallback = newCallback;
}

void* startMovingGhosts()
{

    
    while (running) {

        // logic for chasing only, have to change when implement frightened
        // Utility_sleepForMs(1000);
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
            if(activeGhosts == 0) {
                ghosts[activeGhosts].currentDirection = UP; // the first ghost should be able to move anywhere except down.
            } else {
                ghosts[activeGhosts].currentDirection = LEFT; // entrance is on the left.
            }
            activeGhosts++;
        }
    }
    return NULL;
}

// game manager will call this and pass in the ghost entrance and locations of all ghosts
// i think game manager should start ghost thread because the existence of ghosts depends on the existence of a map?
void Ghost_init(Location ghostEntrance, Location otherGhosts[])
{
    running = 1;
    headStart = 9;
    activeGhosts = 0;
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
    else{
        // printf("Init ghosts...\n");
    }
}

void Ghost_cleanup()
{
    running = 0;
    pthread_join(id, NULL);
}
