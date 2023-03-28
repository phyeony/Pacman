// A game map module
#ifndef _MAP_H_
#define _MAP_H_
#define ROW_SIZE 16
#define COLUMN_SIZE 32
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
    EMPTY
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

typedef struct {
    int row;
    int col;
} Location;

extern Tile empty;
extern Tile pacman;
extern Tile wall;
extern Tile ghost;
extern Tile dot;
extern Tile powerDot;
void Map_init();

#endif