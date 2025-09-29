/**
 * \file posix-download-wifi.c
 *
 * \brief Example of using the library to download instrument data in a POSIX
 * environment over a TCP socket. User must connect to the Logger's Wi-Fi first,
 * this is accomplished by connecting to SSID "RBR ######" where '######' is the 0
 * padded serial number of the device. Once connected this program can be run.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Prerequisite for PATH_MAX in limits.h. */
#define _POSIX_C_SOURCE 200112L

/* Required for errno. */
#include <errno.h>
/* Required for open. */
#include <fcntl.h>
/* Required for PATH_MAX. */
#include <limits.h>
/* Required for fprintf, printf, snprintf. */
#include <stdio.h>
/* Required for strerror. */
#include <string.h>
/* Required for open. */
#include <sys/stat.h>
/* Required for clock_gettime. */
#include <time.h>
/* Required for close, write. */
#include <unistd.h>

#include <sys/time.h>

/* Networking includes */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "posix-shared.h"

#define CHUNK_SIZE 4096
#define HOST_SIZE  1024
static char host[HOST_SIZE];
static uint16_t port = 0;

/**** Private functions.*********/
static int listenUdp(void);
static int openSocketFd(void);
/********************************/

/**
 * Data packets are broadcast over UDP, listen for this to determine if a logger is connected and to
 * obtain busy information and port values.
 */
static int listenUdp(void)
{
    unsigned char message[1024];
    int sock;
    struct sockaddr_in name;
    int bytes;

    printf("Listen for logger.\r\n");

    /* Create socket from which to read */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Opening datagram socket");
        exit(1);
    }

    /* Bind our local address so that the client can send to us */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    name.sin_port = htons(55555);

    if (bind(sock, (struct sockaddr *) &name, sizeof(name))) {
        perror("binding datagram socket");
        exit(1);
    }

    bool messageReceived = false;
    struct sockaddr peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    while (!messageReceived) {

        bytes = recvfrom(sock, message, 1024, 0, &peer_addr, &peer_addr_len);
        printf("nrecv:");
        for (int i = 0; i < bytes; i++) {
            printf("%c", message[i]);
        }

        if (bytes < 110) {
            printf("Wait for a full packet\n");
            continue;
        }

        // get host information.
        char service[64];
        int s = getnameinfo((struct sockaddr *) &peer_addr,
                            peer_addr_len,
                            host,
                            HOST_SIZE,
                            service,
                            64,
                            NI_NUMERICHOST | NI_NUMERICSERV);
        if (s == 0) {
            printf("\nReceived %ld bytes from %s:%s \n", (long) bytes, host, service);
        } else {
            printf("Could not determin host. retry.\n");
            continue;
        }

        // Find 'RBR_' location (in case UDP packet is split and out of order)
        int offset = -1;
        for (int i = 0; i < bytes; i++) {
            if (message[i] == 'R' && message[i + 1] == 'B' && message[i + 2] == 'R' &&
                message[i + 3] == '_' && message[i + 4] == 'W') {
                offset = i;
                break;
            }
        }

        if (offset != 32) {
            printf("Malformed packet. Wait for another.\n");
            continue;
        }

        printf("\r\nDevice ID: %s\n", message + offset);

        // we want to know if the device is busy
        int busy = message[7];
        printf("Device is busy? %d\r\n", busy);
        if (busy == 0) {
            messageReceived = true;
        } else {
            printf("Device is busy. We will wait.\r\n");
        }

        port = (message[8] << 8) | message[9];
        printf("The port is %d\n", port);
    }

    close(sock);

    return 0;
}

/**
 * This function will open the socket conenction to the address and port indicated in the UDP packet
 * to issue commands to the logger. Port and address must have been set by listenUdp before calling
 * this function.
 */
static int openSocketFd(void)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    return sock;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];

    if (argc > 1) {
        fprintf(stderr,
                "%s: No arguments need to be passed in, port and IP are hardcoded!\n",
                programName);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRInstrumentError err;
    RBRInstrument *instrument = NULL;

    // first listen for UDP packets indicating a connection is alive.
    listenUdp();

    // ok, instrument Logger is up and not busy, let's try to connect to socket.
    instrumentFd = openSocketFd();
    if (instrumentFd <= 0) {
        fprintf(stderr, "%s: Failed to open network port: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s (built %s).\n",
            programName,
            RBRINSTRUMENT_LIB_NAME,
            RBRINSTRUMENT_LIB_VERSION,
            RBRINSTRUMENT_LIB_BUILD_DATE);

    RBRInstrumentCallbacks callbacks = {.time = instrumentTime,
                                        .sleep = instrumentSleep,
                                        .read = instrumentRead,
                                        .write = instrumentWrite};

    if ((err = RBRInstrument_open(
             &instrument, &callbacks, INSTRUMENT_COMMAND_TIMEOUT_MSEC, (void *) &instrumentFd)) !=
        RBRINSTRUMENT_SUCCESS) {
        fprintf(stderr,
                "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRInstrumentError_name(err));
        status = EXIT_FAILURE;
        goto socketCleanup;
    }

    printf("Looks like I'm connected to a %s instrument.\n",
           RBRInstrumentGeneration_name(RBRInstrument_getGeneration(instrument)));

    RBRInstrumentId id;
    RBRInstrument_getId(instrument, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.serial,
           id.version);

    RBRInstrumentHardwareRevision hwrev;
    RBRInstrument_getHardwareRevision(instrument, &hwrev);
    printf("It's PCB rev%c, CPU rev%s, BSL v%c.\n", hwrev.pcb, hwrev.cpu, hwrev.bsl);

    RBRInstrumentMemoryInfo meminfo;
    meminfo.dataset = RBRINSTRUMENT_DATASET_STANDARD;
    RBRInstrument_getMemoryInfo(instrument, &meminfo);
    printf("Dataset %s is %0.2f%% full (%" PRIi32 "B used).\n",
           RBRInstrumentDataset_name(meminfo.dataset),
           ((float) meminfo.used) / meminfo.size * 100,
           meminfo.used);

    RBRInstrumentMemoryFormat memformat;
    RBRInstrument_getAvailableMemoryFormats(instrument, &memformat);
    printf("It supports these memory formats:\n");
    for (int i = RBRINSTRUMENT_MEMFORMAT_NONE + 1; i <= RBRINSTRUMENT_MEMFORMAT_MAX; i <<= 1) {
        if (memformat & i) {
            printf("\t%s\n", RBRInstrumentMemoryFormat_name(i));
        }
    }

    RBRInstrument_getCurrentMemoryFormat(instrument, &memformat);
    printf("It's currently storing data of format %s.\n",
           RBRInstrumentMemoryFormat_name(memformat));

    char filename[PATH_MAX + 1];
    snprintf(filename, sizeof(filename), "%06d.bin", id.serial);
    int downloadFd;
    if ((downloadFd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0) {
        fprintf(stderr, "%s: Failed to open output file: %s!\n", programName, strerror(errno));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    struct stat stat;
    if (fstat(downloadFd, &stat) < 0) {
        fprintf(stderr, "%s: Failed to stat output file: %s!\n", programName, strerror(errno));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }
    int32_t initialOffset = stat.st_size;

    if (initialOffset == 0) {
        printf("It looks like the output file, %s, is new. Downloading from "
               "the beginning of instrument memory.\n",
               filename);
    } else {
        printf("It looks like the output file, %s, already contains %" PRIi32
               "B. I'll resume the instrument download from there.\n",
               filename,
               initialOffset);
    }

    uint8_t buf[CHUNK_SIZE];
    RBRInstrumentData data = {.dataset = meminfo.dataset, .offset = initialOffset, .data = buf};

    printf("Downloading:\n");

    struct timespec start;
    struct timespec now;
    double elapsed = 0.0;
    double rate = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (data.offset < meminfo.used) {
        data.size = sizeof(buf);
        err = RBRInstrument_readData(instrument, &data);
        if (err == RBRINSTRUMENT_SUCCESS) {
            write(downloadFd, data.data, data.size);
            data.offset += data.size;
        } else if (err == RBRINSTRUMENT_TIMEOUT) {
            printf("\nWarning: timeout. Retrying...\n");
        } else {
            printf("\nError: %s", RBRInstrumentError_name(err));
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed = now.tv_sec - start.tv_sec;
        elapsed *= 1000000000L;
        elapsed += now.tv_nsec - start.tv_nsec;
        elapsed /= 1000000000L;

        rate = elapsed > 0.0 ? (data.offset - initialOffset) / elapsed : 0.0;

        printf("\r%0.2f%% (%" PRIi32 "B/%" PRIi32 "B; %0.3fs elapsed; "
               "%0.3fB/s)",
               (((float) data.offset) / meminfo.used) * 100,
               data.offset,
               meminfo.used,
               elapsed,
               rate);
    }

    printf("\nDone. Downloaded %" PRIi32 "B in %0.3fs (%0.3fB/s).\n", data.offset, elapsed, rate);

fileCleanup:
    close(downloadFd);
instrumentCleanup:
    RBRInstrument_close(instrument);
socketCleanup:
    close(instrumentFd);

    return status;
}
