#include "gameManager.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Tile gameMap[ROW_SIZE][COLUMN_SIZE];

void GameManager_init()
{
    // Init gameMap
    Tile empty = {EMPTY, BLACK};
    Tile pacman = {PACMAN, YELLOW};
    Tile wall = {WALL, BLUE};
    Tile ghost = {GHOST, RED};
    Tile dot = {DOT, GREEN};
    Tile powerDot = {POWER_DOT, PINK};

    // Reset the screen
    memset(gameMap, 0, sizeof(gameMap));

    Tile mapTopLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall},
        {wall, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, wall},
        {wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall},
        {wall, powerDot, wall, wall, dot, wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall},
        {wall, dot, wall, wall, dot, wall, dot, wall, dot, dot, wall, empty, wall, wall, empty, wall},
        {wall, dot, dot, dot, dot, dot, dot, dot, dot, wall, wall, empty, empty, empty, ghost, empty},
        {wall, wall, wall, dot, wall, wall, wall, dot, wall, wall, wall, empty, wall, wall, wall, wall},
        {empty, empty, empty, dot, dot, dot, dot, dot, dot, dot, empty, empty, wall, ghost, ghost, ghost}};

    Tile mapTopRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall},
        {wall, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, wall},
        {wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall},
        {wall, dot, wall, wall, dot, wall, dot, wall, wall, dot, wall, dot, wall, wall, powerDot, wall},
        {wall, empty, wall, wall, dot, wall, dot, dot, wall, dot, wall, dot, wall, wall, dot, wall},
        {empty, empty, empty, dot, dot, wall, wall, dot, dot, dot, dot, dot, dot, dot, dot, wall},
        {wall, empty, wall, wall, dot, wall, wall, wall, dot, wall, wall, wall, dot, wall, wall, wall},
        {wall, empty, empty, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, empty, empty, empty}};

    Tile mapBottomLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {wall, wall, wall, dot, wall, wall, wall, dot, wall, wall, wall, empty, wall, wall, wall, wall},
        {wall, dot, wall, dot, wall, powerDot, wall, dot, wall, dot, wall, empty, empty, empty, empty, empty},
        {wall, dot, dot, dot, wall, dot, dot, dot, dot, dot, dot, empty, empty, wall, wall, wall},
        {wall, wall, dot, wall, wall, wall, dot, wall, wall, wall, dot, wall, dot, wall, dot, pacman},
        {wall, dot, dot, dot, dot, dot, dot, dot, dot, wall, dot, wall, dot, dot, dot, wall},
        {wall, dot, wall, wall, wall, wall, wall, wall, dot, wall, dot, wall, dot, wall, wall, wall},
        {wall, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot},
        {wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall},
    };

    Tile mapBottomRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {wall, empty, wall, wall, wall, wall, wall, wall, dot, wall, wall, wall, dot, wall, wall, wall},
        {empty, empty, empty, dot, wall, wall, dot, wall, dot, wall, powerDot, wall, dot, wall, dot, wall},
        {wall, wall, wall, dot, dot, dot, dot, dot, dot, dot, dot, wall, dot, dot, dot, wall},
        {dot, dot, wall, dot, wall, dot, wall, wall, wall, dot, wall, wall, wall, dot, wall, wall},
        {wall, dot, dot, dot, wall, dot, wall, dot, dot, dot, dot, dot, dot, dot, dot, wall},
        {wall, wall, wall, dot, wall, dot, wall, dot, wall, wall, wall, wall, wall, wall, dot, wall},
        {dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, dot, wall},
        {wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall, wall}};

    // Merge gameMap parts
    for(int i = 0;i<ROW_SIZE/2;i++){
        for(int j=0;j<COLUMN_SIZE/2;j++){
            gameMap[i][j]=mapTopLeft[i][j];
            gameMap[i][j+COLUMN_SIZE/2]=mapTopRight[i][j];
            gameMap[i+ROW_SIZE/2][j]=mapBottomLeft[i][j];
            gameMap[i+ROW_SIZE/2][j+COLUMN_SIZE/2]=mapBottomRight[i][j];
        }
    }
}

void GameManager_getMap(Tile temp[][COLUMN_SIZE]){
    pthread_mutex_lock(&mutex);
    {
        memcpy(temp, gameMap, ROW_SIZE * COLUMN_SIZE * sizeof(gameMap[0][0]));
    }
    pthread_mutex_unlock(&mutex);
}

void GameManager_cleanup()
{
}