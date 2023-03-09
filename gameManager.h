#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "map.h"
#define ROW_SIZE 16
#define COLUMN_SIZE 32

/**
 *  Display supports only 8(2^3) colours. 
 *  Colours represented in integer is converted to bits in ledDisplay.c. 
 *  Each bit represents one of RGB. For example, WHITE(=7) is converted to [1,1,1] and assigned to [R,G,B] respectively.
 *  Thus, the color is white.
 */
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

void GameManager_init(void);
void GameManager_getMap(Tile map[][COLUMN_SIZE]);
void GameManager_cleanup(void);
void *GameManager_movePacman(Direction direction);

#endif
