#include <stdio.h>
#include <unistd.h>
#include "gameManager.h"
#include "ledDisplay.h"
#include "utility.h"


int main() {
    GameManager_init();
    Utility_sleepForMs(100);
    LedDisplay_init();

    LedDisplay_cleanup();
    GameManager_cleanup();

    return 0;
}