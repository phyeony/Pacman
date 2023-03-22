// Shutdown.h
// Module to lock main thread until shutdown is triggered.
#ifndef _SHUTDOWN_H_
#define _SHUTDOWN_H_

void Shutdown_cleanup();
void Shutdown_waitUntilShutdown();
void Shutdown_init();

#endif
