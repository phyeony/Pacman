#include "ghost.h"
#include "utility.h"
#include "types.h"
#include "map.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static Ghost ghosts[GHOST_NUM]={
    {.id = 0, .name = "ghostA", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.id = 1, .name = "ghostB", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.id = 2, .name = "ghostC", .mode = PAUSED, .currentDirection = IDLE_STATE},
    {.id = 3, .name = "ghostD", .mode = PAUSED, .currentDirection = IDLE_STATE},
};
static int activeGhosts = 0;
static int running = 1;
static int headStart = 9;

static pthread_t id;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* startMovingGhosts();
static GhostCallback movementCallback;
static GhostColorChangeCallback changeGhostColorCallback;

Ghost *Ghost_getGhostAtLocation(Location targetLocation) {
    // location is initialized in Ghost_init and GameManagmer_moveGhost
    for (int i = 0; i < GHOST_NUM; i++) {
        if (ghosts[i].location.row == targetLocation.row && ghosts[i].location.col == targetLocation.col) {
            return &ghosts[i];
        }
    }
    return NULL;
}

void Ghost_changeAllGhostMode(GhostMode newMode) {
    for (int i = 0; i < GHOST_NUM; i++) {
        ghosts[i].mode = newMode;
        if(newMode == FRIGHTENED) {
            ghosts[i].modeStartTimeInMs = Utility_getCurrentTimeInMs();
        }
    }
}

void Ghost_registerCallback(GhostCallback newCallback, GhostColorChangeCallback newColorCallback)
{
    movementCallback = newCallback;
    changeGhostColorCallback = newColorCallback;
}

void Ghost_decreaseActiveGhostCount() {
    pthread_mutex_lock(&mutex);
    {
        activeGhosts--;
    }
    pthread_mutex_unlock(&mutex);
}

void* startMovingGhosts()
{

    Utility_sleepForMs(1000); 
    while (running) {
        int tempActiveGhosts = 0;
        // pthread_mutex_lock(&mutex);
        // {
            tempActiveGhosts = activeGhosts;
        // }
        // pthread_mutex_unlock(&mutex);
        // logic for chasing only, have to change when implement frightened
        for (int i = 0; i < tempActiveGhosts; i++){
            if(ghosts[i].mode == FRIGHTENED) {
                printf("Time Passed: %lld, Time Left: %lld \n", Utility_getCurrentTimeInMs() - ghosts[i].modeStartTimeInMs, FRIGHTENED_DURATION_MS - (Utility_getCurrentTimeInMs() - ghosts[i].modeStartTimeInMs));
            }
            if(ghosts[i].mode == FRIGHTENED &&  Utility_getCurrentTimeInMs() - ghosts[i].modeStartTimeInMs > FRIGHTENED_DURATION_MS - FRIGHTENED_GHOST_TRANSITION_DURATION_MS) {
                (*changeGhostColorCallback)(WHITE);
                Utility_sleepForMs(100);
                (*changeGhostColorCallback)(RED);
                Utility_sleepForMs(100);
                (*changeGhostColorCallback)(WHITE);
                Utility_sleepForMs(100);
                // TODO: NOT SURE IF THIS WILL WORK.
            }   
            if(ghosts[i].mode == FRIGHTENED && Utility_getCurrentTimeInMs() - ghosts[i].modeStartTimeInMs > FRIGHTENED_DURATION_MS) {
                (*changeGhostColorCallback)(RED);
                Utility_sleepForMs(150);
                ghosts[i].mode = CHASE;
                ghosts[i].modeStartTimeInMs = 0;
            }
            // if(ghosts[i].mode == PAUSED && Utilty_)
            (*movementCallback)(&ghosts[i]);
            Utility_sleepForMs(GHOST_SPEED_DELAY);
        }
        if (tempActiveGhosts < 4){
            headStart++;
        }
        if (headStart == 10){
            headStart = 0;
            if(ghosts[tempActiveGhosts].mode == PAUSED) {
                ghosts[tempActiveGhosts].mode = CHASE;
            }
            if(ghosts[tempActiveGhosts].id == 0) {
                ghosts[tempActiveGhosts].currentDirection = IDLE_STATE; // the first ghost should be able to move anywhere except down.
            } else {
                ghosts[tempActiveGhosts].currentDirection = LEFT; // entrance is on the left.
            }
            pthread_mutex_lock(&mutex);
            {
                activeGhosts++;
            }
            pthread_mutex_unlock(&mutex);

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
