#include "shutdown.h"
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static bool isShutdownExiting = false;

void Shutdown_cleanup() {
    printf("Cleanup Shutdown...\n");

    pthread_mutex_lock(&mutex);
    isShutdownExiting = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

// lock main thread until cleanup is triggered.
void Shutdown_waitUntilShutdown(){
    pthread_mutex_lock(&mutex);

    while (!isShutdownExiting) {
        pthread_cond_wait(&cond, &mutex);
    }

    pthread_mutex_unlock(&mutex);
}
