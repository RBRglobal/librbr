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
/* Required for memmove, strerror. */
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
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "posix-shared.h"

/* Note: Using a larger chunk size (e.g. 34000 or 68000) can improve download
 * throughput significantly on a good connection. */
#define CHUNK_SIZE 34000
#define HOST_SIZE 1024

/* Rolling throughput window. */
#define NSEC_PER_SEC 1000000000LL
#define ROLLING_WINDOW_NSEC (10LL * NSEC_PER_SEC)
#define ROLLING_MAX_SAMPLES 1024

/* Number of times a single chunk will be re-requested before giving up. */
#define MAX_CHUNK_RETRIES 5

typedef struct ThroughputSample
{
    /* Timestamp when the chunk of bytes finished downloading. */
    struct timespec ts;
    /* Byte offset of the start of the chunk that was downloaded. */
    int32_t offset;
} ThroughputSample;

static char host[HOST_SIZE];
static uint16_t port = 0;

/* Samples for calculating rolling throughput over the period specified by
 * ROLLING_WINDOW_NSEC. Ordered oldest-first. Entries which have not been written
 * yet are zeroed and will be ignored. */
static ThroughputSample rollingSamples[ROLLING_MAX_SAMPLES];

/**** Private functions.*********/
static int listenUdp( void );
static int openSocketFd( void );
static void rollingPush(struct timespec ts, int32_t offset);
static double rollingRateBps(struct timespec now, int32_t currentOffset);
static bool isRetriableDownloadError(RBRGen3Error err);
/********************************/

/**
 * Records a throughput sample.
 *
 * Shifts every existing sample one place towards the left of the array,
 * discarding the oldest, and stores the new (timestamp, byte offset) pair in 
 * the final element. Keeping the samples in oldest-first order lets
 * rollingRateBps() scan them in a single pass.
 */
static void rollingPush(struct timespec ts, int32_t offset)
{
    memmove(&rollingSamples[0],
            &rollingSamples[1],
            sizeof(rollingSamples) - sizeof(rollingSamples[0]));
    rollingSamples[ROLLING_MAX_SAMPLES - 1].ts = ts;
    rollingSamples[ROLLING_MAX_SAMPLES - 1].offset = offset;
}

/**
 * Computes the rolling download throughput in bytes per second.
 *
 * Finds the oldest sample which still falls within the ROLLING_WINDOW_NSEC
 * window, then divides the number of bytes downloaded since that sample by the
 * elapsed time. This yields an average rate over (approximately) the trailing 
 * window rather than over the entire download.
 */
static double rollingRateBps(struct timespec now, int32_t currentOffset)
{
    const long long nowNs = (long long) now.tv_sec * NSEC_PER_SEC + now.tv_nsec;
    const long long thresholdNs = nowNs - ROLLING_WINDOW_NSEC;

    /* Scan from the oldest sample, skipping those which have fallen out of the
     * window. Entries which have never been written hold a zero timestamp; they
     * are outside the window too, but only once the machine has been up for
     * ROLLING_WINDOW_NSEC, so exclude them explicitly rather than relying on
     * thresholdNs being positive. */
    for (int i = 0; i < ROLLING_MAX_SAMPLES; i++)
    {
        const ThroughputSample *const sample = &rollingSamples[i];
        const long long tsNs = (long long) sample->ts.tv_sec * NSEC_PER_SEC
                               + sample->ts.tv_nsec;
        if (tsNs == 0 || tsNs <= thresholdNs)
        {
            continue;
        }

        const double dt = (nowNs - tsNs) / 1e9;
        return dt > 0.0 ? (currentOffset - sample->offset) / dt : 0.0;
    }

    return 0.0;
}

/**
 * Data packets are broadcast over UDP, listen for this to determine if a logger is connected and to obtain 
 * busy information and port values.
 */
static int listenUdp( void )
{
    unsigned char message[1024];
    int sock;
    struct sockaddr_in name;
    int bytes;

    printf("Listen for logger.\r\n");

    /* Create socket from which to read */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)   {
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
    socklen_t peer_addr_len  = sizeof(struct sockaddr_storage);
    while (!messageReceived)
    {

        bytes = recvfrom(sock, message, 1024, 0, &peer_addr, &peer_addr_len) ;
            printf("nrecv:");
            for (int i = 0; i < bytes; i++)
            {
                printf("%c", message[i]);
            }

        if (bytes < 110)
        {
            printf("Wait for a full packet\n");
            continue;
        }       

        //get host information.        
        char service[64];
        int s = getnameinfo((struct sockaddr *) &peer_addr,
                        peer_addr_len, host, HOST_SIZE,
                        service, 64, NI_NUMERICHOST | NI_NUMERICSERV);
        if (s == 0)
        {
            printf("\nReceived %ld bytes from %s:%s \n", (long) bytes, host, service);
        }
        else
        {
            printf("Could not determin host. retry.\n");
            continue;
        }

        //Find 'RBR_' location (in case UDP packet is split and out of order)
        int offset = -1;
        for(int i = 0 ; i < bytes; i++)
        {
            if (message[i] == 'R' && message[i+1] == 'B' && message[i+2] == 'R' && message[i+3] == '_' && message[i+4] == 'W')
            {
                offset = i;
                break;
            }
        }

        if (offset != 32)
        {
            printf("Malformed packet. Wait for another.\n");
            continue;
        }

        printf("\r\nDevice ID: %s\n", message + offset);

        //we want to know if the device is busy 
        int busy = message[7];           
            printf("Device is busy? %d\r\n", busy);
            if (busy == 0)
            {
                messageReceived = true;        
            }
            else
            {
                printf("Device is busy. We will wait.\r\n");
            }

        port = (message[8] << 8) | message[9];
        printf("The port is %d\n", port);
    }
    
    close(sock);

    return 0;
}

/**
 * This function will open the socket conenction to the address and port indicated in the UDP packet to issue commands to the logger.
 * Port and address must have been set by listenUdp before calling this function. 
 */
static int openSocketFd( void )
{
    int sock = 0; 
    struct sockaddr_in serv_addr; 

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) 
    { 
        printf("Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, host, &serv_addr.sin_addr)<=0)  
    { 
        printf("Invalid address/ Address not supported \n"); 
        return -1; 
    } 
 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    /* Set a short timeout for failing to receive a responses from the 
     * instrument before triggering a re-request.
     */
    static const struct timeval recvTimeout = { 
        .tv_sec = INSTRUMENT_CHARACTER_TIMEOUT_MSEC / 1000, 
        .tv_usec = (INSTRUMENT_CHARACTER_TIMEOUT_MSEC % 1000) * 1000,
    };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout));

    /* This is a command/response protocol: we send tiny commands and expect a
     * reply. Nagle's algorithm holds back a small segment while an earlier
     * small segment is still unacknowledged, so if a command packet is lost,
     * our re-request commands queue up locally instead of going on the wire
     * (they get coalesced into one packet once the stuck segment is finally
     * ACKed). That also starves the duplicate ACKs that would otherwise trigger
     * a fast retransmit, leaving us to wait out the full (RTT-variance-inflated)
     * RTO. Disabling Nagle lets each re-request hit the wire immediately, so a
     * lost command can be fast-retransmitted in ~1 RTT instead of seconds.
     */
    static const int optionEnabled = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optionEnabled, sizeof(optionEnabled));

    /* This connection is a "thin stream" (very few packets in flight at once),
     * which is exactly what Linux's thin-stream options were added for.
     * TCP_THIN_DUPACK triggers a fast retransmit after a single duplicate ACK
     * instead of three (so recovery can happen on the first re-request), and
     * TCP_THIN_LINEAR_TIMEOUTS avoids exponential RTO backoff. Both are
     * Linux-specific, so guard them for portability.
     */
#ifdef TCP_THIN_DUPACK
    setsockopt(sock, IPPROTO_TCP, TCP_THIN_DUPACK,
               &optionEnabled, sizeof(optionEnabled));
#endif
#ifdef TCP_THIN_LINEAR_TIMEOUTS
    setsockopt(sock, IPPROTO_TCP, TCP_THIN_LINEAR_TIMEOUTS,
               &optionEnabled, sizeof(optionEnabled));
#endif
    return sock;
}

/**
 * Whether a readData error is the kind we can recover from by re-requesting the
 * same chunk. These all indicate a transport-level problem with the chunk
 * itself (short/garbled data) rather than something fatal to the connection.
 */
static bool isRetriableDownloadError(RBRGen3Error err)
{
    return err == RBRGEN3_TIMEOUT
           || err == RBRGEN3_CALLBACK_ERROR
           || err == RBRGEN3_CHECKSUM_ERROR
           || err == RBRGEN3_COMMUNICATION_ERROR;
}

int main(int argc, char *argv[])
{   
    char *programName = argv[0];

    if (argc > 1)
    {
        fprintf(stderr, "%s: No arguments need to be passed in, port and IP are hardcoded!\n", programName);
        return EXIT_FAILURE;
    }
 
    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 *conn = NULL;
    #ifdef RBR_LIB_NODYNAMICMEMORYALLOCATION
    RBRGen3 instrumentSpace;
    conn = &instrumentSpace;
    #endif

    //first listen for UDP packets indicating a connection is alive.
    listenUdp();

    //ok, instrument Logger is up and not busy, let's try to connect to socket.
    instrumentFd = openSocketFd();
    if (instrumentFd <= 0)
    {
        fprintf(stderr, "%s: Failed to open network port: %s!\n", programName, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s: Using %s v%s.\n",
            programName,
            RBRGEN3_LIB_NAME,
            RBRGEN3_LIB_VERSION);

    RBRGen3Callbacks callbacks = {
        .time = instrumentTime,
        .sleep = instrumentSleep,
        .read = instrumentRead,
        .write = instrumentWrite
    };

    if ((err = RBRGen3_open(
             &conn,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN3_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto socketCleanup;
    }

    printf(
        "Looks like I'm connected to a %s instrument.\n",
        RBRGen3Generation_name(RBRGen3_getGeneration(conn)));

    RBRGen3Id id;
    RBRGen3_getId(conn, &id);
    printf("The instrument is an %s (fwtype %d), serial number %06d, with "
           "firmware v%s.\n",
           id.model,
           id.fwtype,
           id.serial,
           id.version);

    RBRGen3HardwareRevision hwrev;
    RBRGen3_getHardwareRevision(conn, &hwrev);
    printf("It's PCB rev%c, CPU rev%s, BSL v%c.\n",
           hwrev.pcb,
           hwrev.cpu,
           hwrev.bsl);

    RBRGen3MemoryInfo meminfo;
    meminfo.dataset = RBRGEN3_DATASET_STANDARD;
    RBRGen3_getMemoryInfo(conn, &meminfo);
    printf("Dataset %s is %0.2f%% full (%" PRIi32 "B used).\n",
           RBRGen3Dataset_name(meminfo.dataset),
           ((double) meminfo.used) / meminfo.size * 100,
           meminfo.used);

    RBRGen3MemoryFormat memformat;
    RBRGen3_getAvailableMemoryFormats(conn, &memformat);
    printf("It supports these memory formats:\n");
    for (int i = RBRGEN3_MEMFORMAT_NONE + 1;
         i <= RBRGEN3_MEMFORMAT_MAX;
         i <<= 1)
    {
        if (memformat & i)
        {
            printf("\t%s\n", RBRGen3MemoryFormat_name(i));
        }
    }

    RBRGen3_getCurrentMemoryFormat(conn, &memformat);
    printf("It's currently storing data of format %s.\n",
           RBRGen3MemoryFormat_name(memformat));

    char filename[PATH_MAX + 1];
    snprintf(filename, sizeof(filename), "%06d.bin", id.serial);
    int downloadFd;
    if ((downloadFd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
    {
        fprintf(stderr, "%s: Failed to open output file: %s!\n",
                programName,
                strerror(errno));
        status = EXIT_FAILURE;
        goto instrumentCleanup;
    }

    struct stat stat;
    if (fstat(downloadFd, &stat) < 0)
    {
        fprintf(stderr, "%s: Failed to stat output file: %s!\n",
                programName,
                strerror(errno));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }
    int32_t initialOffset = stat.st_size;

    if (initialOffset == 0)
    {
        printf("It looks like the output file, %s, is new. Downloading from "
               "the beginning of instrument memory.\n",
               filename);
    }
    else
    {
        printf("It looks like the output file, %s, already contains %" PRIi32
               "B. I'll resume the instrument download from there.\n",
               filename,
               initialOffset);
    }

    uint8_t buf[CHUNK_SIZE];
    RBRGen3Data data = {
        .dataset = meminfo.dataset,
        .offset  = initialOffset,
        .data    = buf
    };

    printf("Downloading:\n");

    struct timespec start;
    struct timespec now;
    double elapsed = 0.0;
    double rate = 0.0;
    double rollingRate = 0.0;
    int chunkRetries = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (data.offset < meminfo.used)
    {
        data.size = sizeof(buf);
        err = RBRGen3_readData(conn, &data);
        if (err != RBRGEN3_SUCCESS)
        {
            if (isRetriableDownloadError(err) && chunkRetries < MAX_CHUNK_RETRIES)
            {
                chunkRetries++;
                printf("\n%s at offset %" PRIi32 "B; re-requesting chunk "
                    "(attempt %d of %d)...\n",
                    RBRGen3Error_name(err),
                    data.offset,
                    chunkRetries,
                    MAX_CHUNK_RETRIES);
                continue;
            } else {
                printf("\nError: %s", RBRGen3Error_name(err));
                break;
            }
        }

        write(downloadFd, data.data, data.size);
        data.offset += data.size;
        chunkRetries = 0;

        clock_gettime(CLOCK_MONOTONIC, &now);

        elapsed  = now.tv_sec - start.tv_sec;
        elapsed *= NSEC_PER_SEC;
        elapsed += now.tv_nsec - start.tv_nsec;
        elapsed /= NSEC_PER_SEC;

        rate = elapsed > 0.0 ? (data.offset - initialOffset) / elapsed : 0.0;

        rollingPush(now, data.offset);
        rollingRate = rollingRateBps(now, data.offset);

        printf("\r%0.2f%% (%" PRIi32 "B/%" PRIi32 "B; %0.3fs elapsed; "
               "%0.3fB/s avg; %0.3fB/s 10s rolling)",
               (((double) data.offset) / meminfo.used) * 100,
               data.offset,
               meminfo.used,
               elapsed,
               rate,
               rollingRate);
    }

    printf("\nDone. Downloaded %" PRIi32 "B in %0.3fs (%0.3fB/s).\n",
           data.offset,
           elapsed,
           rate);

fileCleanup:
    close(downloadFd);
instrumentCleanup:
    RBRGen3_close(conn);
socketCleanup:
    close(instrumentFd);

    return status;
}
