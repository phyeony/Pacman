#include "gameManager.h"
#include "ghost.h"
#include "map.h"
#include "ledDisplay.h"
#include "joycon.h"
#include "zenCapeJoystick.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <limits.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Tile gameMap[ROW_SIZE][COLUMN_SIZE];
static Location pacmanLocation;
void ghostChase();
int isValid(Location loc);
Location dijkstra(Location ghostLoc, Location pacmanLoc);

// populate these later as we populate game map
static Location ghostHouseEntrance;
static Location ghostHouse[GHOST_NUM-1];


// Init gameMap


static int foodCollected = 0;

// Prototypes for helper functions
int checkCollision(Location loc);
void checkOutOfBounds(Location* loc);
// static void GameManager_moveGhost(Ghost *);

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
        {empty, empty, empty, dot, dot, dot, dot, dot, dot, dot, empty, empty, empty, ghost,ghost,ghost}};

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
    int ghostCount = 0;
    for (int i = 0; i<ROW_SIZE;i++){
        for (int j = 0; j<COLUMN_SIZE;j++){
            if (gameMap[i][j].tileType==PACMAN){
                pacmanLocation.row = i;
                pacmanLocation.col = j;
            }
            if (ghostCount < GHOST_NUM && gameMap[i][j].tileType==GHOST){
                if (!ghostCount){
                    ghostHouseEntrance.row = i;
                    ghostHouseEntrance.col = j;
                    ghostCount++;
                    continue;
                }
                ghostHouse[ghostCount - 1].row = i;
                ghostHouse[ghostCount - 1].col = j;
                ghostCount++;
            }
        }
    }

    // Init Ghost
    Ghost_init(ghostHouseEntrance,ghostHouse);
    Ghost_registerCallback(&GameManager_moveGhost);
    Joycon_registerCallback(&GameManager_movePacman, &GameManager_cleanup);
    ZenCapeJoystick_registerCallback(&GameManager_movePacman);
    LEDDisplay_registerCallback(&GameManager_getMap);

}

void GameManager_getMap(Tile temp[][COLUMN_SIZE])
{
    pthread_mutex_lock(&mutex);
    {
        memcpy(temp, gameMap, ROW_SIZE * COLUMN_SIZE * sizeof(gameMap[0][0]));
    }
    pthread_mutex_unlock(&mutex);
}

void GameManager_moveGhost(Ghost* currentGhost) 
{
    ghostChase(currentGhost);

    // switch (currentGhost->mode)
    // {
    //     case CHASE:
    //         break;
    //     case FRIGHTENED:
    //         // Algo. Check the timer.
    //         break;
    //     case PAUSED:
    //         // move
    //         break;
    //     default:
    //         break;
    // }
}

void ghostChase(Ghost* currentGhost)
{
    Location ghostLocation = currentGhost->location;
    Tile currentTile = currentGhost->currentTile;
    printf("Current tile %d\n",currentGhost->currentTile.tileType);
    // calculate new ghost location using Dijkstra's algorithm
    Location newGhostLoc = dijkstra(ghostLocation, pacmanLocation);
    // move ghost towards new location
    pthread_mutex_lock(&mutex);
    {
        gameMap[ghostLocation.row][ghostLocation.col] = currentTile;
        ghostLocation = newGhostLoc;
        currentTile = gameMap[newGhostLoc.row][newGhostLoc.col];
        gameMap[newGhostLoc.row][newGhostLoc.col] = ghost;
        // printf("tileType:%d\n",gameMap[ghostLocation.row][ghostLocation.col].tileType);
    }
    pthread_mutex_unlock(&mutex);
    
    // check for collision with pacman
    if (ghostLocation.row == pacmanLocation.row && ghostLocation.col == pacmanLocation.col) {
        printf("Game over! Ghost caught Pacman.\n");
        GameManager_cleanup();
    }
    currentGhost->location = ghostLocation;
    currentGhost->currentTile = currentTile;
}

// moves Pacman in static game map. 
void GameManager_movePacman(Direction direction)
{
    Location newLoc;
    // printf("Move pacman: %d\n",direction);
    // edge cases: moving out of bounds, moving to next map segment
    if (direction == UP){
        newLoc.col = pacmanLocation.col;
        newLoc.row = pacmanLocation.row-1;
    }
    else if (direction == DOWN){
        newLoc.col = pacmanLocation.col;
        newLoc.row = pacmanLocation.row+1;
    }
    else if (direction == LEFT){
        newLoc.col = pacmanLocation.col-1;
        newLoc.row = pacmanLocation.row;
    }
    else if (direction == RIGHT){
        newLoc.col = pacmanLocation.col+1;
        newLoc.row = pacmanLocation.row;
    }
    // printf("new loc:%d,%d!\n",newLoc.row,newLoc.col);
    checkOutOfBounds(&newLoc);
    int collision = checkCollision(newLoc);
    if (collision==WALL){
        // printf("wall!\n");
        return;
    }
    if (collision == GHOST){
        printf("Ghost caught pacman, game over!\n");
        return;
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
    return;
}
int checkCollision(Location loc)
{
    Tile tile;
    pthread_mutex_lock(&mutex);
    {
        tile = gameMap[loc.row][loc.col];
    }
    pthread_mutex_unlock(&mutex);

    if (tile.tileType == WALL){
        return WALL;
    }
    if (tile.tileType == DOT){
        return DOT;
    }
    if (tile.tileType == GHOST){
        return GHOST;
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





int isValid(Location loc) {
    // Check if location is within bounds of game map
    if (loc.row < 0 || loc.row >= ROW_SIZE || loc.col < 0 || loc.col >= COLUMN_SIZE) {
        return 0;
    }
    if (gameMap[loc.row][loc.col].tileType == WALL || gameMap[loc.row][loc.col].tileType == GHOST) {
        return 0;
    }
    return 1;
}

Location dijkstra(Location ghostLoc, Location pacmanLoc) {
    int dist[ROW_SIZE][COLUMN_SIZE];
    int visited[ROW_SIZE][COLUMN_SIZE];
    Location prev[ROW_SIZE][COLUMN_SIZE];

    // initialize distances and visited array
    for (int i = 0; i < ROW_SIZE; i++) {
        for (int j = 0; j < COLUMN_SIZE; j++) {
            dist[i][j] = INT_MAX;
            visited[i][j] = 0;
        }
    }

    // set distance to the ghost's location to 0
    dist[ghostLoc.row][ghostLoc.col] = 0;

    // find shortest path using Dijkstra's algorithm
    for (int count = 0; count < ROW_SIZE * COLUMN_SIZE - 1; count++) {
        int minDist = INT_MAX;
        Location u;

        // find the vertex with the minimum distance
        for (int i = 0; i < ROW_SIZE; i++) {
            for (int j = 0; j < COLUMN_SIZE; j++) {
                if (!visited[i][j] && dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    u.row = i;
                    u.col = j;
                }
            }
        }

        // mark the vertex as visited
        visited[u.row][u.col] = 1;

        // update the distance of adjacent vertices
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) {
                    continue;
                }

                int row = u.row + i;
                int col = u.col + j;

                if (row >= 0 && row < ROW_SIZE && col >= 0 && col < COLUMN_SIZE) {
                    int altDist = dist[u.row][u.col] + 1;
                    Location loc = {row,col};
                    pthread_mutex_lock(&mutex);
                    if (altDist < dist[row][col] && isValid(loc)) {
                        dist[row][col] = altDist;
                        prev[row][col] = u;
                    }
                    pthread_mutex_unlock(&mutex);
                }
            }
        }
    }

    // backtrack to find the shortest path to Pacman
    Location nextMove = pacmanLoc;
    while (prev[nextMove.row][nextMove.col].row != ghostLoc.row || prev[nextMove.row][nextMove.col].col != ghostLoc.col) {
        nextMove = prev[nextMove.row][nextMove.col];
    }
    
    return nextMove;
}

void GameManager_cleanup()
{
    Ghost_cleanup();
}