#include "gameManager.h"
#include "ghost.h"
#include "map.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Tile gameMap[ROW_SIZE][COLUMN_SIZE];
static Location pacmanLocation;

static const Location ghostHouseEntrance = { 5, 14 };
static const Location ghostHouse[GHOST_NUM-1] = {{7,13}, {7,14}, {7,15}};
static Ghost ghosts[GHOST_NUM]={
    {.name = "ghostA", .mode = PAUSED,},
    {.name = "ghostB", .mode = PAUSED,},
    {.name = "ghostC", .mode = PAUSED,},
    {.name = "ghostD", .mode = PAUSED,},
};

// Init gameMap
static Tile empty = {EMPTY, BLACK};
static Tile pacman = {PACMAN, PINK};
static Tile wall = {WALL, BLUE};
static Tile ghost = {GHOST, RED};
static Tile dot = {DOT, GREEN};
static Tile powerDot = {POWER_DOT, YELLOW};

static int foodCollected = 0;

// Prototypes for helper functions
static int compareTiles(Tile tile1, Tile tile2);
int checkCollision(Location loc);
void checkOutOfBounds(Location* loc);
static void GameManger_moveGhost(Ghost *);

void GameManager_init()
{
    // Reset the screen
    memset(gameMap, 0, sizeof(gameMap));
    // pacman start position: mapBottomLeft[4][15]
    // 
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
    for (int i = 0; i<ROW_SIZE;i++){
        for (int j = 0; j<COLUMN_SIZE;j++){
            if (compareTiles(gameMap[i][j],pacman)){
                pacmanLocation.row = i;
                pacmanLocation.col = j;
            }
        }
    }

    //Init Ghost
    ghosts[0].location = ghostHouseEntrance;
    for(int i =1; i <GHOST_NUM; i++) {
        ghosts[i].location = ghostHouse[i-1];
    }
    Ghost_registerCallback(ghosts, GHOST_NUM, &GameManger_moveGhost);
}



void GameManager_getMap(Tile temp[][COLUMN_SIZE])
{
    pthread_mutex_lock(&mutex);
    {
        memcpy(temp, gameMap, ROW_SIZE * COLUMN_SIZE * sizeof(gameMap[0][0]));
    }
    pthread_mutex_unlock(&mutex);
}

static void GameManger_moveGhost(Ghost *currentGhost) 
{
    switch (currentGhost->mode)
    {
        case CHASE:
            // do something
            break;
        case FRIGHTENED:
            // Algo. Check the timer.
            break;
        case PAUSED:
            // move
            break;
        default:
            break;
    }
}


// moves Pacman in static game map. 
void* GameManager_movePacman(Direction direction)
{
    Location newLoc;
    // printf("Move pacman: %d\n",direction);
    // edge cases: moving out of bounds, moving to next map segment
    if (direction == UP){
        newLoc.col = pacmanLocation.col;
        newLoc.row = pacmanLocation.row+1;
    }
    else if (direction == DOWN){
        newLoc.col = pacmanLocation.col;
        newLoc.row = pacmanLocation.row-1;
    }
    else if (direction == LEFT){
        newLoc.col = pacmanLocation.col+1;
        newLoc.row = pacmanLocation.row;
    }
    else if (direction == RIGHT){
        newLoc.col = pacmanLocation.col-1;
        newLoc.row = pacmanLocation.row;
    }
    // printf("new loc:%d,%d!\n",newLoc.row,newLoc.col);
    checkOutOfBounds(&newLoc);
    int collision = checkCollision(newLoc);
    if (collision==WALL){
        // printf("wall!\n");
        return NULL;
    }
    if (collision==DOT||collision==EMPTY){
        pthread_mutex_lock(&mutex);
        {
            gameMap[pacmanLocation.row][pacmanLocation.col] = empty;
            pacmanLocation = newLoc;
            gameMap[newLoc.row][newLoc.col] = pacman;
            // printf("tileType:%d\n",gameMap[pacmanLocation.row][pacmanLocation.col].tileType);
        }
        pthread_mutex_unlock(&mutex);
    }
    if (collision==DOT){
        foodCollected++;
    }
    return NULL;
}
int checkCollision(Location loc)
{
    Tile tile;
    pthread_mutex_lock(&mutex);
    {
        tile = gameMap[loc.row][loc.col];
    }
    pthread_mutex_unlock(&mutex);

    if (compareTiles(tile, wall)){
        return WALL;
    }
    if (compareTiles(tile,dot)){
        return DOT;
    }
    return EMPTY;
}
void checkOutOfBounds(Location* loc)
{
    if (loc->row == ROW_SIZE) {
        loc->row = 0;
    }
    else if (loc->row == -1){
        loc->row = ROW_SIZE-1;
    }
    else if (loc->col == COLUMN_SIZE){
        loc->col = 0;
    }
    else if (loc->col==-1){
        loc->col = COLUMN_SIZE-1;
    }
}
int compareTiles(Tile tile1, Tile tile2)
{
    if (tile1.tileType == tile2.tileType){
        return 1;
    }
    return 0;
}
void GameManager_cleanup()
{
}