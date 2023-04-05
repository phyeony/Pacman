#ifndef _JOYCON_H_
#define _JOYCON_H_

#include "utility.h"
#include "types.h" // shared types
#define VENDOR_ID 0x045e
#define PRODUCT_ID 0x028e
#define INTERFACE_NUMBER 0
#define ENDPOINT 0x81

typedef void (*JoyconCallback)(Direction direction);
void* Joycon_init(void);
void Joycon_cleanup(void);
void Joycon_quitGame(void);
void Joycon_registerCallback(JoyconCallback callback);
#endif