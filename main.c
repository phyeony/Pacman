#include <stdio.h>
#include <unistd.h>
#include "gameManager.h"
#include "ledDisplay.h"
#include "utility.h"
#include "joycon.h"
#include "ghost.h"
int main() {
    GameManager_init();
    Utility_sleepForMs(100);
    LedDisplay_init();
    Joycon_init();
    Ghost_init();
    LedDisplay_cleanup();
    GameManager_cleanup();
    Joycon_cleanup();
    Ghost_cleanup();
    return 0;
}