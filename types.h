// types.h
#ifndef _TYPES_H_
#define _TYPES_H_
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    IDLE_STATE,
    BUTTON_B,
    BUTTON_Y
} Direction;

typedef struct {
    int row;
    int col;
} Offset;

#endif