#ifndef _LEDDISPLAY_H_
#define _LEDDISPLAY_H_
#include "map.h"
typedef void (*LEDDisplayCallback)(Tile temp[][COLUMN_SIZE], int *currentScore, int *highScore);
void LedDisplay_init(void);
void LedDisplay_cleanup(void);
void LedDisplay_quitGame();
void LEDDisplay_registerCallback(LEDDisplayCallback getMapCb);
#endif
