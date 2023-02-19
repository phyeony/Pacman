#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#define ROW_SIZE 16
#define COLUMN_SIZE 32

/**
 *  Display supports only 8(2^3) colours. 
 *  Colours represented in integer is converted to bits in ledDisplay.c. 
 *  Each bit represents one of RGB. For example, WHITE(=7) is converted to [1,1,1] and assigned to [R,G,B] respectively.
 *  Thus, the color is white.
 */
typedef enum {
    BLACK=0,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PINK,
    LIGHT_BLUE,
    WHITE,
    END,  // This will display BLACK.
} Color;

typedef enum
{
    WALL,
    PACMAN,
    DOT,
    POWER_DOT,
    GHOST,
    EMPTY
} TileType;

typedef struct
{
    TileType tileType;
    Color color;
} Tile;

void GameManager_init(void);
void GameManager_getMap(Tile map[][COLUMN_SIZE]);
void GameManager_cleanup(void);

#endif
