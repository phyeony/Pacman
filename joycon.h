#ifndef _JOYCON_H_
#define _JOYCON_H_
#define VENDOR_ID 0x045e
#define PRODUCT_ID 0x028e
#define INTERFACE_NUMBER 0
#define ENDPOINT 0x81
void* Joycon_init(void);
void Joycon_cleanup(void);

#endif