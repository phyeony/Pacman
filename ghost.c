#include "ghost.h"
//#include <pthread.h>

//Local headers
// static void *startMovingGhosts(void *args);

// static int running;
// static pthread_t id;

void Ghost_registerCallback(Ghost ghosts[], int size, GhostCallback newCallback)
{
    for(int i=0; i<size; i++) {
        ghosts[i].movementCallback = newCallback;
    }
}

// void Ghost_init(Ghost ghost, GhostCallback callback)
// {
    
// }

// static void *startMovingGhosts(void *args)
// {
//     // while(running){
//     //     for(int i=0; i<GHOST_NUM; i++) {
//     //         GameManager_moveGhost(ghosts[i].mode, ghosts[i].location);
//     //     }
//     // }
// }

// void Ghost_init()
// {
//     running = 1;
//     for(int i=0; i< GHOST_NUM; i++){
//         ghosts[i].mode = PAUSED;
//         ghosts[i].modeDurationTime = 1000;
//     }
//     ghosts[0].location = GameManager_getGhostEntrance();
//     // Ask this from GameManger as well? Is there no circular dependencies?

//     if (pthread_create(&id, NULL, &startMovingGhosts, NULL) != 0)
//     {
//         perror("Failed to create a new thread...");
//     }
// }

// void Ghost_cleanup()
// {
//     running = 0;
//     pthread_join(id, NULL);
// }
