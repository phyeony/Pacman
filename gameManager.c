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
#include <time.h>

// Local headers
static Location dijkstra(Location ghostLoc, Location pacmanLoc);
static int isValidPathway(int row, int col);
static int isOutOfBound(int row, int col);
static int isIntersection(int row, int col);
static Direction getNewDirectionFromLocations(Location currentLocation, Location newLoc);
static Location chooseRandomValidPath(int row, int col);
static Direction oppositeDirection(Direction dir);
static TileType getCollidingTileType(int row, int col);
static void moveGhostBackToGhostHouse(Ghost *ghostP);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Tile gameMap[ROW_SIZE][COLUMN_SIZE];
static Location pacmanLocation;

// populate these later as we populate game map
static Location firstGhostLocation;
static Location ghostHouse[GHOST_NUM-1];
static Location intersections[MAX_INTERSECTION_SIZE];
static int intersectionsCount = 0;
static int totalFoodCount = 0;
static Offset offsets[] = {
    [UP] = { .row = -1, .col = 0 },
    [DOWN] = { .row = 1, .col = 0 },
    [LEFT] = { .row = 0, .col = -1 },
    [RIGHT] = { .row = 0, .col = 1 },
};

static int highScore = 0;
static int currentScore = 0;
static int gameOver = 0;
// Prototypes for helper functions
int checkCollision(Location loc);
void checkOutOfBounds(Location* loc);

void GameManager_init(int first_init)
{
    srand(time(NULL));   // Initialization, should only be called once.
    GameManager_initializeMap();
    printf("foodcount %d\n",totalFoodCount);
    Joycon_registerCallback(&GameManager_movePacman);
    ZenCapeJoystick_registerCallback(&GameManager_movePacman);
    LEDDisplay_registerCallback(&GameManager_getMap);
}

void GameManager_initializeMap(){
    gameOver = 0;
    totalFoodCount = 0;
    currentScore = 0;
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
    int pathways = 0;   
    //Tile inter = { .tileType=EMPTY, WHITE}; just to test intersection
    for (int i = 0; i<ROW_SIZE;i++){
        for (int j = 0; j<COLUMN_SIZE;j++){
            if (gameMap[i][j].tileType==PACMAN){
                pacmanLocation.row = i;
                pacmanLocation.col = j;
            }
            if (gameMap[i][j].tileType==DOT || gameMap[i][j].tileType==POWER_DOT ){
                totalFoodCount++;
            }
            if (ghostCount < GHOST_NUM && gameMap[i][j].tileType==GHOST){
                if (!ghostCount){
                    firstGhostLocation.row = i;
                    firstGhostLocation.col = j;
                    ghostCount++;
                } else {
                    ghostHouse[ghostCount - 1].row = i;
                    ghostHouse[ghostCount - 1].col = j;
                    ghostCount++;
                }
            }
            if(gameMap[i][j].tileType != WALL) {
                // Extract intersection with more than 2 pathways
                if(i+1 < ROW_SIZE && gameMap[i+1][j].tileType != WALL) {
                    pathways++;
                }
                if(i-1 >= 0 && gameMap[i-1][j].tileType != WALL) {
                    pathways++;
                }
                if(j-1 >=0 && gameMap[i][j-1].tileType != WALL) {
                    pathways++;
                }
                if(j+1 < COLUMN_SIZE && gameMap[i][j+1].tileType != WALL) {
                    pathways++;
                }
                if(pathways > 2) {
                    // this is intersection
                    intersections[intersectionsCount].row = i;
                    intersections[intersectionsCount].col = j;
                    intersectionsCount++;
                 
                    // gameMap[i][j] = inter; just to test intersection
                }
                pathways = 0;
            }
        }
    }
    // Init Ghost
    Ghost_init(firstGhostLocation, ghostHouse);
    Ghost_registerCallback(&GameManager_moveGhost, &GameManager_changeAllGhostColor);
}

void GameManager_gameover()
{
    Tile mapTopLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, ghost, ghost, ghost, ghost, empty, empty, empty, ghost, empty, empty, empty, ghost, ghost, ghost},
        {empty, ghost, empty, empty, empty, ghost, empty, empty, ghost, empty, ghost, empty, empty, ghost, empty, ghost},
        {empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, ghost},
        {empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, ghost},
        {empty, ghost, empty, empty, ghost, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, ghost, empty, empty},
        {empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, empty},
        {empty, empty, ghost, ghost, ghost, ghost, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty}};

    Tile mapTopRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {ghost, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty}};

    Tile mapBottomLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, ghost, ghost, ghost, ghost, empty, ghost, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, empty, ghost},
        {empty, empty, empty, empty, empty, empty, empty, empty, ghost, ghost, ghost, ghost, ghost, empty, empty, empty}};

    Tile mapBottomRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, ghost, ghost, ghost, ghost, empty, empty},
        {empty, empty, ghost, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty},
        {empty, empty, ghost, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty},
        {empty, empty, ghost, empty, ghost, ghost, ghost, ghost, ghost, empty, ghost, ghost, ghost, ghost, empty, empty},
        {empty, empty, ghost, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, ghost, empty, empty, empty},
        {empty, ghost, empty, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, ghost, empty, empty},
        {ghost, empty, empty, empty, ghost, ghost, ghost, ghost, ghost, empty, ghost, empty, empty, empty, ghost, empty}};

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

void GameManager_win()
{
    Tile mapTopLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, ghost, ghost, ghost},
        {empty, empty, empty, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, empty, ghost},
        {empty, empty, empty, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, empty, ghost},
        {empty, empty, empty, empty, ghost, empty, empty, ghost, empty, empty, ghost, empty, empty, empty, empty, ghost}};

    Tile mapTopRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {ghost, ghost, empty, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, ghost, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, ghost, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, ghost, empty, empty, ghost, empty, empty, empty, empty, empty, empty, empty, empty}};

    Tile mapBottomLeft[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, ghost, empty, empty, ghost, empty, empty, ghost, empty, empty, empty, empty, ghost},
        {empty, empty, empty, empty, ghost, empty, empty, ghost, empty, empty, ghost, empty, empty, empty, empty, ghost},
        {empty, empty, empty, empty, empty, ghost, ghost, empty, ghost, ghost, empty, empty, empty, ghost, ghost, ghost},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty}};

    Tile mapBottomRight[ROW_SIZE / 2][COLUMN_SIZE / 2] = {
        {empty, empty, empty, empty, ghost, empty, empty, empty, ghost, empty, ghost, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, ghost, empty, empty, empty, empty, ghost, ghost, empty, empty, empty, empty, empty},
        {ghost, ghost, empty, empty, ghost, empty, empty, empty, empty, empty, ghost, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty},
        {empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty}};

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

void GameManager_getData(Tile temp[][COLUMN_SIZE], int *currentScoreP, int *highScoreP)
{
    pthread_mutex_lock(&mutex);
    {
        memcpy(temp, gameMap, ROW_SIZE * COLUMN_SIZE * sizeof(gameMap[0][0]));
        *currentScoreP = currentScore;
        *highScoreP = highScore;
    }
    pthread_mutex_unlock(&mutex);
}



Direction directions[] = {UP, RIGHT, DOWN, LEFT};

static Direction oppositeDirection(Direction dir) {
    switch (dir) {
        case UP:    return DOWN;
        case RIGHT: return LEFT;
        case DOWN:  return UP;
        case LEFT:  return RIGHT;
        default:    return IDLE_STATE;
    }
}

void printScore(){
    gameOver = 1;
    pthread_mutex_lock(&mutex);
    {
        if (currentScore > highScore){
            highScore = currentScore;
            printf("New high score: %d\n",highScore);
        }
    }
    pthread_mutex_unlock(&mutex);
    if (currentScore > highScore){
        highScore = currentScore;
    }
    else {
        printf("Your score: %d\nHigh score: %d\n",currentScore,highScore);
    }
    printf("Would you like to restart? Press Y to accept, otherwise press B to exit game\n");
}

void GameManager_moveGhost(Ghost* currentGhost) 
{
    // TODO: Currently, if 2 ghosts are coming towards from a different direction, they will collide and stay stuck forever.
    //       Need to fix this.
    int row = currentGhost->location.row;
    int col = currentGhost->location.col;
    Direction direction = currentGhost->currentDirection;
    Direction newDirection = IDLE_STATE;
    Tile newTile;
    Location newLoc = {2,2}; // This location should not be used if everything is working as expected.

    // Step 1: Get the new Ghost location
    // * Move along the path if the ghost is not on interection with more than 3 valid
    if(!isIntersection(row, col)) {
        // printf("Not intersection! :%s\n", currentGhost->name);
        int surroundingGhosts = 0; 

        // Check all possible directions
        for (int i = 0; i < IDLE_STATE; i++) {
            int newRow = row + offsets[i].row;
            int newCol = col + offsets[i].col;
            Direction tempNewDir = i;

            if (isOutOfBound(newRow, newCol) || direction == oppositeDirection(tempNewDir)) {
                continue;
            }

            TileType type = getCollidingTileType(newRow, newCol);

            if (type == WALL) {
                continue;
            } else if (type == GHOST) {
                if (tempNewDir == direction) {
                    // Case 2: Another ghost is in front of it. The current ghost stays.
                    return;
                }
                surroundingGhosts++;
            } else {
                // Case 1: Ghost goes to a new tile that is empty.
                newLoc.row = newRow;
                newLoc.col = newCol;
                newDirection = tempNewDir;
                break;
            }
        }

        if (newDirection == IDLE_STATE) {
            if (surroundingGhosts == 1) {
                // Case 4: Ghost is surrounded by 3 walls and a ghost. The current ghost stays.
                return;
            } else {
                // Case 3: All 3 paths are blocked by walls. Ghost goes back to the tile that is the opposite of its current direction.
                newLoc.row = row + offsets[oppositeDirection(direction)].row;
                newLoc.col = col + offsets[oppositeDirection(direction)].col;
                newDirection = oppositeDirection(direction);
            }
        } 
        if (newLoc.row == 2 && newLoc.col ==2 ) {
            printf("ERROR: IN NOT INTERSEcTION%s\n", currentGhost->name);
            return;
        }
    } else {
        // printf("Intersection! :%s\n", currentGhost->name);
        // * On intersection, perform each mode's algorithm to get the new Ghost location
        switch (currentGhost->mode)
        {
            case CHASE:
                newLoc = dijkstra(currentGhost->location, pacmanLocation);
                break;
            case FRIGHTENED:
                newLoc = chooseRandomValidPath(row, col);
                break;
            // case PAUSED:
            //     // move
            //     break;
            default:
                //printf("ON INTERSEC NO CHASE OR FRIGHTENED MODE??");
                return;
                break;
        }
        if (newLoc.row == 2 && newLoc.col ==2 ) {
            printf("ERROR: IN INTERSEcTION%s\n", currentGhost->name);
            return;
        }
        newDirection = getNewDirectionFromLocations(currentGhost->location, newLoc);
    }
    if (newLoc.row == 2 && newLoc.col ==2 ) {
        printf("ERROR: Somewhere else%s\n", currentGhost->name);
        return;
    }   

    // Step 2: Move ghost towards new location
    pthread_mutex_lock(&mutex);
    {
        gameMap[row][col] = currentGhost->currentTile;
        newTile = gameMap[newLoc.row][newLoc.col];
        gameMap[newLoc.row][newLoc.col] = ghost;
        currentGhost->location = newLoc;
        currentGhost->currentDirection = newDirection;
        currentGhost->currentTile = newTile;
    }
    pthread_mutex_unlock(&mutex);
 

    // Step 3: Check for collision with pacman
    if (newLoc.row == pacmanLocation.row && newLoc.col == pacmanLocation.col) {
        if(currentGhost->mode == FRIGHTENED) {
            printf("Ghost is caught!\n");
            moveGhostBackToGhostHouse(currentGhost);
            // TODO: Go back to ghost house.
        } else {
            printf("Game over! Ghost caught Pacman.\n");
            printScore();
            Ghost_cleanup();
            GameManager_gameover();
        }
    }
}

static int isIntersection(int row, int col) {
    for(int i = 0; i < intersectionsCount; i++) {
        if(row == intersections[i].row && col == intersections[i].col) {
            return 1;
        }
    }
    return 0;
}

static Direction getNewDirectionFromLocations(Location currentLocation, Location newLoc) {
    if(newLoc.row - currentLocation.row == 1) {
        return DOWN;
    } else if (newLoc.row - currentLocation.row == -1) {
        return UP;
    } else if (newLoc.col - currentLocation.col == 1) {
        return RIGHT;
    } else {
        return LEFT;
    }
}

void GameManager_changeAllGhostColor(Color newColor)
{
    ghost.color=newColor;
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
    else if (direction == BUTTON_B){
        GameManager_cleanup();
        return;
        // exit game turn off screen
    }
    else if (direction == BUTTON_Y && gameOver){
       GameManager_initializeMap();
       return;
    }
    else {
        return;
    }
    // printf("new loc:%d,%d!\n",newLoc.row,newLoc.col);
    checkOutOfBounds(&newLoc);
    int collision = checkCollision(newLoc);
    if (collision==WALL){
        // printf("wall!\n");
        return;
    }
    if (collision == GHOST){
        // Do different thing depending on the ghost mode.
        Ghost *ghostP = Ghost_getGhostAtLocation(newLoc);
        if(ghostP==NULL) {
            printf("ERROR: SHOULD NOT HAPPEN\n");
        }
        if (ghostP->mode == FRIGHTENED) {
            printf("Ghost is caught!\n");
            // TODO: Go back to ghost house.
            updateCurrentScore(500);
            moveGhostBackToGhostHouse(ghostP);
        } else if (ghostP-> mode == CHASE) {
            printf("Game over! Ghost caught Pacman.\n");
            printScore();
            Ghost_cleanup();
            GameManager_gameover();
            //GameManager_cleanup();
        }
        return;
    }
    if (collision == POWER_DOT){
        printf("POWER - special.\n");
        updateCurrentScore(150);

        totalFoodCount--;
        // for testing - just eat 11 dots to win, full implementation win when total = 0
        if (totalFoodCount==0){
            printf("\n\nYou won!!\n");
            printScore();
            Ghost_cleanup();
            GameManager_win();
        } else {
            Ghost_changeAllGhostMode(FRIGHTENED);
            ghost.color=WHITE;
        }
    }

        
    if (collision==DOT||collision==EMPTY||collision==POWER_DOT){
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
        totalFoodCount--;
        updateCurrentScore(100);
        // for testing - just eat 11 dots to win, full implementation win when total = 0
        if (totalFoodCount==0){
            printf("\n\nYou won!!\n");
            printScore();
            Ghost_cleanup();
            GameManager_win();
        }
    }

    return;
}

static void updateCurrentScore(int addition) {
    pthread_mutex_lock(&mutex);
    {
        currentScore += addition;
    }
    pthread_mutex_unlock(&mutex);
}

static void moveGhostBackToGhostHouse(Ghost *ghostP) {
   
    Location newLoc;
    if(ghostP->id ==0) {
        newLoc = firstGhostLocation;
    } else {
        newLoc = ghostHouse[(ghostP->id)-1];
    }
    pthread_mutex_lock(&mutex);
    {
        // what if the ghost was on dot ?
        gameMap[ghostP->location.row][ghostP->location.col] = ghostP->currentTile;
        gameMap[newLoc.row][newLoc.col] = ghost;

        // might be able to get away without mutex.
        ghostP->location = newLoc;
        ghostP->mode = FRIGHTENED;
        if(ghostP->id == 0) {
            ghostP->currentDirection = IDLE_STATE;
        } else {
            ghostP->currentDirection = LEFT;
        }
        ghostP->currentTile = empty;
        //Ghost_decreaseActiveGhostCount();
    }
    pthread_mutex_unlock(&mutex);
}

int checkCollision(Location loc)
{
    Tile tile;
    pthread_mutex_lock(&mutex);
    {
        tile = gameMap[loc.row][loc.col];
    }
    pthread_mutex_unlock(&mutex);
    return tile.tileType;
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



static int isOutOfBound(int row, int col) {
    return (row < 0 || row >= ROW_SIZE || col < 0 || col >= COLUMN_SIZE);
}

static TileType getCollidingTileType(int row, int col) {
    // Check if location is within bounds of game map
    if (isOutOfBound(row, col)) {
        return OUT_OF_BOUND;
    }
    TileType temp;
    pthread_mutex_lock(&mutex);
    temp = gameMap[row][col].tileType;
    pthread_mutex_unlock(&mutex);
    return temp;
}

static int isValidPathway(int row, int col) {
    if (isOutOfBound(row,col)) {
        return 0;
    }
    TileType type = getCollidingTileType(row, col);
    if(type==WALL || type== GHOST){
        return 0;
    }
    return 1;
}

static Location chooseRandomValidPath(int row, int col) {
    Location validPaths[MAX_NUM_VALID_PATHWAYS-1];
    int pathwayCount = 0;
    // collect valid pathways
    for (int dir = 0; dir < IDLE_STATE; dir++) {
        int newRow = row + offsets[dir].row;
        int newCol = col + offsets[dir].col;

        if (isValidPathway(newRow, newCol)) {
            validPaths[pathwayCount] = (Location) {newRow, newCol};
            pathwayCount++;
        }
    }
    /* random int between 0 and pathwayCount-1*/
    int randInt = rand() % pathwayCount;
    return validPaths[randInt];
}

static Location dijkstra(Location ghostLoc, Location pacmanLoc) {
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

        // check if the visited vertex is Pacman's location
        if (u.row == pacmanLoc.row && u.col == pacmanLoc.col) {
            break;
        }

        // update the distance of adjacent vertices
        for (int dir = 0; dir < IDLE_STATE; dir++) {
            int row = u.row + offsets[dir].row;
            int col = u.col + offsets[dir].col;

            if (row >= 0 && row < ROW_SIZE && col >= 0 && col < COLUMN_SIZE) {
                int altDist = dist[u.row][u.col] + 1;
                if (altDist < dist[row][col] && isValidPathway(row, col)) {
                    dist[row][col] = altDist;
                    prev[row][col] = u;
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
    LedDisplay_quitGame();
    Ghost_cleanup();
}