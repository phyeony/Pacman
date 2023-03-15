#ifndef _JOYCON_H_
#define _JOYCON_H_

#include "utility.h"
#define VENDOR_ID 0x045e
#define PRODUCT_ID 0x028e
#define INTERFACE_NUMBER 0
#define ENDPOINT 0x81
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;
typedef void (*JoyconCallback)(Direction direction);
typedef void (*ShutdownCallback)();
void* Joycon_init(void);
void Joycon_cleanup(void);
void Joycon_registerCallback(JoyconCallback callback, ShutdownCallback shutdown);
#endif