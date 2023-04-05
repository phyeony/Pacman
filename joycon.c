#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include "joycon.h"
static pthread_t joyconThreadId;
static int running = 1;
static void *joyconThread();
static libusb_context *ctx;
static libusb_device_handle *dev_handle;
static ssize_t cnt;
static JoyconCallback cb;
// this function is to be called by gameManager to pass in a function pointer that moves pacman
void Joycon_registerCallback(JoyconCallback onJoyconInput)
{
    cb = onJoyconInput;
}

void *Joycon_init(void)
{
    int error;
    error = libusb_init(&ctx);
    if (error != LIBUSB_SUCCESS)
    {
        fprintf(stderr, "libusb_init() failed: %s\n", libusb_strerror(error));
        return NULL;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (dev_handle == NULL)
    {
        fprintf(stderr, "libusb_open_device_with_vid_pid() failed: device not found\n");
        libusb_exit(ctx);
        return NULL;
    }
    if (dev_handle != NULL)
    {
        libusb_detach_kernel_driver(dev_handle, INTERFACE_NUMBER);
        {
            error = libusb_claim_interface(dev_handle, 0);
            if (error != LIBUSB_SUCCESS)
            {
                fprintf(stderr, "libusb_claim_interface() failed: %s\n", libusb_strerror(error));
                libusb_close(dev_handle);
                libusb_exit(ctx);
                return NULL;
            }
        }
    }

    printf("Waiting for controller input...\n");
    pthread_create(&joyconThreadId, NULL, joyconThread, NULL);
    return NULL;
}

void Joycon_cleanup(void)
{
    printf("stopping joycon thread\n");
    running = 0;
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    pthread_join(joyconThreadId, NULL);
}
void Joycon_quitGame(void)
{
    printf("stopping joycon thread\n");
    running = 0;
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    pthread_cancel(joyconThreadId);
}

void *joyconThread()
{
    unsigned char data[20];

    while (1)
    {
        int error = libusb_interrupt_transfer(dev_handle, ENDPOINT, data, sizeof(data), &cnt, 0);
        if (error == LIBUSB_ERROR_INTERRUPTED)
        {
            break;
        }
        if (error != LIBUSB_SUCCESS)
        {
            fprintf(stderr, "libusb_interrupt_transfer() failed: %s\n", libusb_strerror(error));
            break;
        }
        // print hex values
        // for (int i = 0; i < 20; i++)
        // {
        //     printf("%x ", data[i]);
        // }
        // printf("\n");
        if (data[3] == 0x20)
        {
            printf("exit pressed\n");
            (*cb)(BUTTON_B);
        }
        if (data[3] == 0x80)
        {
            (*cb)(BUTTON_Y);
            printf("Y pressed\n");
        }
        if (data[8] == 0xff && data[9] == 0x7f)
        {
            (*cb)(UP);
            // printf("up\n");
        }
        if (data[7] == 0x80)
        {
            (*cb)(LEFT);
            // printf("left\n");
        }
        if (data[6] == 0xff && data[7] == 0x7f)
        {
            (*cb)(RIGHT);
            // printf("right\n");
        }
        if (data[9] == 0x80)
        {
            (*cb)(DOWN);
            // printf("down\n");
        }
        Utility_sleepForMs(150);
    }

    return 0;
}
