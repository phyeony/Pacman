#include <stdio.h>
#include <unistd.h>
#include "gameManager.h"
#include "ledDisplay.h"
#include "utility.h"
#include "joycon.h"
#include "zenCapeJoystick.h"
#include "ghost.h"
#include "udp.h"
#include "shutdown.h"

int main() {
    GameManager_init();
    Utility_sleepForMs(100);
    LedDisplay_init();
    Joycon_init();
    ZenCapeJoystick_init();
    Ghost_init();
    Udp_init();

    Shutdown_waitUntilShutdown();
    
    Udp_cleanup();
    LedDisplay_cleanup();
    GameManager_cleanup();
    ZenCapeJoystick_cleanup();
    Joycon_cleanup();
    Ghost_cleanup();

    Shutdown_cleanup();
    return 0;
}