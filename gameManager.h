#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "map.h"
#include "joycon.h"


/**
 *  Display supports only 8(2^3) colours. 
 *  Colours represented in integer is converted to bits in ledDisplay.c. 
 *  Each bit represents one of RGB. For example, WHITE(=7) is converted to [1,1,1] and assigned to [R,G,B] respectively.
 *  Thus, the color is white.
 */


void GameManager_init(int first_init);
void GameManager_initializeMap();
void GameManager_cleanup(void);
void GameManager_movePacman(Direction direction);
void GameManager_moveGhost();
void GameManager_changeAllGhostColor(Color color);
void GameManager_getMap(Tile map[][COLUMN_SIZE]);
void GameManager_getScores(int *currentScore, int *highScore);
int GameManager_isGameOver();

#endif
