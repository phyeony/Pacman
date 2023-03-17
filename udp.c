#include "udp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <errno.h>
#include "utility.h"
#include <stdbool.h>
#include "shutdown.h"
#include <arpa/inet.h>
#include "gameManager.h"
#include "map.h"

#define PORT 12345
#define RECEIVER_PORT 3000
#define MAX_LEN 28700
#define INT_BASE 10
#define TEMP_ITEM_LENGTH 350
#define SAMPLE_ITEM_SIZE 7
#define PACKET_MAX_SIZE 1500

static pthread_t id;
static struct sockaddr_in sinLocal;
static struct sockaddr_in sinRemote;
static int socketDescriptor;
static unsigned int sin_len = sizeof(sinRemote);
static bool exitUdp = false;

// Local hearders
static void *terminate(char *);
static void *updateVolume(char *);
static void *updateData(char *);
static void *healthCheck(char *);
static void setUpSocket(void);
static void *startUdpProgram(void *);
static void replyToMessage(char *);

typedef enum
{
    UPDATE_TEMPO = 0,
    UPDATE_VOLUME,
    UPDATE_DRUM_BEAT_MODE,
    PLAY_DRUM_SOUND,
    TERMINATE,
    UPDATE_DATA,
    HEALTH_CHECK,
    UNKNOWN,
} CommandName;

typedef struct Command
{
    char *commandNameInString;
    void *(*action)(char *);
} Command;

static char messageToReply[MAX_LEN];

static Command commands[] = {
    [UPDATE_VOLUME] = {"updateVolume", updateVolume},
    [TERMINATE] = {"terminate", terminate},
    [UPDATE_DATA] = {"updateData", updateData},
    [HEALTH_CHECK] = {"healthcheck", healthCheck},
    [UNKNOWN] = {"unknwon command", NULL}
};


// args = "true", "false"
static void *updateVolume(char *args)
{
    if(strcmp(args, "true")==0) {
        // do smth
    } else {
        // do smth
    }
    // snprintf(messageToReply, MAX_LEN, "updateVolumeAnswer %d", getVolume());
    return NULL;
}

// args = " "
static void *updateData(char *args)
{
    char answerString = "updateMapAnswer ";
    size_t answerStringLength = strlen(answerString);
    int gameMap[ROW_SIZE][COLUMN_SIZE];
    GameManager_getMap(gameMap);
    // flattens 2d array to 1d array
    uint8_t data[ROW_SIZE * COLUMN_SIZE * sizeof(uint8_t)];
    memcpy(data, gameMap, sizeof(data));

    memcpy(messageToReply, answerString, answerStringLength);
    memcpy(messageToReply + answerStringLength, data, sizeof(data));
    return NULL;
}


// args = " "
static void *healthCheck(char *args)
{
    // call shutdown module
    snprintf(messageToReply, MAX_LEN, "Chealthcheck true");
    return NULL;
}


// args = " "
static void *terminate(char *args)
{
    // call shutdown module
    Shutdown_cleanup();
    snprintf(messageToReply, MAX_LEN, "Terminating..");
    return NULL;
}

/*
- sockaddr_in: Socket Address for INternet (struct)
- sinLocal: Socket INternet, such as in sin_family
- AF_INET: Address Family, Internet (IPV4)
- PF_INET: Protocol Family, Internet (IPV4)
- SOCK_DGRAM: Socket, user Datagram protocol (UDP)
*/

static void setUpSocket()
{
    // address structure
    memset(&sinLocal, 0, sizeof(sinLocal));
    sinLocal.sin_family = AF_INET;
    sinLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    sinLocal.sin_port = htons(PORT);

    // create socket
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor < 0)
    {
        perror("Failed to create socket. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    // bind socket
    if (bind(socketDescriptor, (struct sockaddr *)&sinLocal, sizeof(sinLocal)) < 0)
    {
        fprintf(stderr, "Failed to bind socket to PORT:%d Exiting...\n", PORT);
        perror("");
        close(socketDescriptor);
        exit(EXIT_FAILURE);
    }
}

// NOTE: received message has following format
// [command] [arguments]
// Examples:
// 1. updateVolume true  => Here, true is a value for "increase". If true, increase the value. Else, decrease the value.
static void replyToMessage(char *messageReceived)
{
    memset(messageToReply, 0, sizeof(messageToReply));
    // Is it bad since they're initialized every loop?
    Command command = commands[UNKNOWN];
    char temp[MAX_LEN];
    strncpy(temp, messageReceived, MAX_LEN);
    // poor error handling
    char *firstSubString = strtok(temp, " ");
    char *secondSubString = strtok(NULL, " \n");

    //Don't loop REPEAT
    for (int i = 0; i < UNKNOWN; i++)
    {
        if(firstSubString==NULL) {
            // Unknown command
            break;
        }
        if (strcmp(firstSubString, commands[i].commandNameInString) == 0)
        {
            command = commands[i];
        }
    }
    // Execute action callback with argumnet
    command.action(secondSubString);

    // Here starts the logic to send out mutliple packets if the message is bigger than the packet's maximum size.
    sin_len = sizeof(sinRemote); // not sure if this is needed. might've already been assigned in recvfrom.
    int bytesLeftInReply = 0;
    for(int i=0; i<MAX_LEN; i++) {
        if(messageToReply[i]=='\0'){
            break;
        }
        bytesLeftInReply++;
    }

    if(bytesLeftInReply < PACKET_MAX_SIZE) {
        strncat(messageToReply, "\n", MAX_LEN -1 -strnlen(messageToReply,MAX_LEN));
        sendto(socketDescriptor, messageToReply, strlen(messageToReply), 0, (struct sockaddr *)&sinRemote, sin_len);
        return;
    } else {
        // when message is too big to be sent in one packet, send it in multiple packets.
        char smallPacket[PACKET_MAX_SIZE];
        memset(smallPacket, 0, sizeof(smallPacket));

        int numItemsInOnePacket = PACKET_MAX_SIZE/SAMPLE_ITEM_SIZE;
        int packetSize = SAMPLE_ITEM_SIZE * numItemsInOnePacket;
        int sendCount=0;
        while(bytesLeftInReply != 0) {
            if(bytesLeftInReply > packetSize) {
                memcpy(smallPacket, messageToReply+sendCount*packetSize, packetSize);
                bytesLeftInReply= bytesLeftInReply - packetSize;
                sendto(socketDescriptor, smallPacket, strlen(smallPacket), 0, (struct sockaddr *)&sinRemote, sin_len);
            } else {
                char lastPacket[bytesLeftInReply+2];
                memset(lastPacket, 0, sizeof(lastPacket));
                memcpy(lastPacket, messageToReply+sendCount*packetSize, bytesLeftInReply);
                strncat(lastPacket, "\n", bytesLeftInReply+2 -1 -strnlen(lastPacket,bytesLeftInReply+2));
                sendto(socketDescriptor, lastPacket, strlen(lastPacket), 0, (struct sockaddr *)&sinRemote, sin_len);
                bytesLeftInReply=0;
            }
            sendCount++;
        }
    }
}

static void *startUdpProgram(void *args)
{
    // why is this needed???
    Utility_sleepForMs(100);
    setUpSocket();
    char messageReceived[MAX_LEN];
    memset(messageReceived, 0, sizeof(messageReceived));

    while (!exitUdp)
    {
        // Receive message from the binded port
        int bytesRx = recvfrom(socketDescriptor, messageReceived, MAX_LEN - 1, 0, (struct sockaddr *)&sinRemote, &sin_len);
        if (bytesRx < 0)
        {
            perror("Failed to receive message. Exiting...\n");
            close(socketDescriptor);
            exit(EXIT_FAILURE);
        }
        sinRemote.sin_port = htons(RECEIVER_PORT);
        // Null terminated (string)
        messageReceived[bytesRx] = 0;

        replyToMessage(messageReceived);
    }
    return NULL;
}

void Udp_init()
{
    printf("Start udp...\n");
    pthread_create(&id, NULL, &startUdpProgram, NULL);
}

void Udp_cleanup()
{
    printf("Cleanup udp...\n");
    exitUdp = true;
   
    close(socketDescriptor);
    shutdown(socketDescriptor, SHUT_RDWR);
    pthread_join(id, NULL);
}
