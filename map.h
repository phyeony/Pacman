// A game map module
#ifndef _MAP_H_
#define _MAP_H_
#define ROW_SIZE 16
#define COLUMN_SIZE 32
#define MAX_INTERSECTION_SIZE 150
#define MAX_NUM_VALID_PATHWAYS 4
typedef enum 
{
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
    EMPTY,
    OUT_OF_BOUND
} TileType;

typedef enum
{
    topLeft,
    topRight,
    bottomLeft,
    bottomRight
} MapSegment;

typedef struct
{
    TileType tileType;
    Color color;
} Tile;

extern Tile empty;
extern Tile pacman;
extern Tile wall;
extern Tile ghost;
extern Tile dot;
extern Tile powerDot;
void Map_init();

#endif