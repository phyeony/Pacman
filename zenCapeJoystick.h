// A joystick module that reads the input and respond with actions on a background thread.
#ifndef _ZENCAPEJOYSTICK_H_
#define _ZENCAPEJOYSTICK_H_

#include "types.h"

typedef void (*JoystickCallback)(Direction direction);
void ZenCapeJoystick_init(void);
void ZenCapeJoystick_cleanup(void);
void ZenCapeJoystick_registerCallback(JoystickCallback callback);

#endif