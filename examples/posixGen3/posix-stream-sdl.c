/**
 * \file posix-stream-sdl.c
 *
 * \brief Example of using the library to stream instrument data to a GUI
 * application in a POSIX environment.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for errno. */
#include <errno.h>
/* Required for DBL_MAX. */
#include <float.h>
/* Required for SDL_*. */
#include <SDL2/SDL.h>
/* Required for fprintf. */
#include <stdio.h>
/* Required for memcpy, strerror. */
#include <string.h>
/* Required for close. */
#include <unistd.h>

#include "posix-shared.h"

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))

#define SAMPLE_SIZE (60 * 12)

#define VER_PAD 32

static int PLOT_COLORS[][3] = {
    {0xFF, 0x33, 0x33},
    {0x33, 0x33, 0xFF},
    {0x00, 0x80, 0x80},
    {0xFF, 0x80, 0x80},
    {0x00, 0x00, 0x80},
    {0x00, 0x80, 0x00},
    {0x80, 0x00, 0x80},
    {0x00, 0x80, 0x80},
    {0x80, 0x80, 0x80},
    {0x80, 0x80, 0x00},
    {0x80, 0x00, 0x80},
    {0x33, 0xFF, 0x33},
    {0xFF, 0x55, 0xFF},
    {0x80, 0x00, 0x00},
    {0xc0, 0x00, 0x00},
    {0x00, 0x00, 0xC0},
    {0x00, 0xC0, 0x00},
    {0x40, 0x40, 0x40},
    {0xFF, 0x40, 0x40},
    {0x40, 0x40, 0xFF},
    {0x40, 0xFF, 0x40},
    {0xFF, 0x40, 0xFF}
};
#define PLOT_COLORS_LEN (sizeof(PLOT_COLORS) / sizeof(PLOT_COLORS[0]))

static RBRGen3Sample callbackSample;
static RBRGen3Sample *samples = NULL;
static SDL_Point *samplePoints[RBRGEN3_CHANNEL_MAX] = {
    NULL
};
static int sampleCount = 0;
static int samplePointChannelCount = 0;
static int samplePointCount = 0;

static SDL_Window *window;
static SDL_Renderer *renderer;

RBRGen3Error instrumentSample(
    const struct RBRGen3 *instrument,
    const struct RBRGen3Sample *const sample)
{
    /* Unused. */
    (void) instrument;

    if (sampleCount == SAMPLE_SIZE)
    {
        memmove(&samples[0],
                &samples[1],
                (sampleCount-- *sizeof(RBRGen3Sample)));
    }

    memcpy(&samples[sampleCount++], sample, sizeof(RBRGen3Sample));

    return RBRGEN3_SUCCESS;
}

static void recalculatePoints(void)
{
    int width;
    int height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    RBRGen3DateTime minTime = samples[0].timestamp,
                          maxTime = samples[sampleCount - 1].timestamp;
    RBRGen3DateTime duration = maxTime - minTime;
    double horScale = ((double) width) / duration;

    samplePointChannelCount = RBRGEN3_CHANNEL_MAX;
    double minVal[RBRGEN3_CHANNEL_MAX] = {
        DBL_MAX
    };
    double maxVal[RBRGEN3_CHANNEL_MAX] = {
        DBL_MIN
    };
    for (int i = 0; i < sampleCount; i++)
    {
        samplePointChannelCount = MIN(samples[i].channels,
                                      samplePointChannelCount);
        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            minVal[channel] = MIN(samples[i].readings[channel],
                                  minVal[channel]);
            maxVal[channel] = MAX(samples[i].readings[channel],
                                  maxVal[channel]);
        }
    }
    double verScale[RBRGEN3_CHANNEL_MAX];
    for (int channel = 0; channel < samplePointChannelCount; channel++)
    {
        verScale[channel] = ((double) (height - 2 * VER_PAD)) / (maxVal[channel] - minVal[channel]);
    }

    for (int i = 0; i < sampleCount; i++)
    {
        RBRGen3DateTime timestamp = samples[i].timestamp - minTime;
        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            double value = samples[i].readings[channel] - minVal[channel];
            samplePoints[channel][i].x = timestamp * horScale;
            samplePoints[channel][i].y = height - (value * verScale[channel]) - VER_PAD;
        }
    }
    samplePointCount = sampleCount;
}

int main(int argc, char *argv[])
{
    char *programName = argv[0];
    char *devicePath;

    int status = EXIT_SUCCESS;
    int instrumentFd;

    RBRGen3Error err;
    RBRGen3 *instrument = NULL;

    if ((samples = malloc(sizeof(RBRGen3Sample) * SAMPLE_SIZE)) == NULL)
    {
        fprintf(stderr,
                "%s: Failed to allocate sample buffer!\n",
                programName);
        return EXIT_FAILURE;
    }

    for (int channel = 0; channel < RBRGEN3_CHANNEL_MAX; channel++)
    {
        if ((samplePoints[channel] = malloc(sizeof(SDL_Point) * SAMPLE_SIZE))
            == NULL)
        {
            fprintf(stderr,
                    "%s: Failed to allocate point buffer %d!\n",
                    programName,
                    channel + 1);
            return EXIT_FAILURE;
        }
    }

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s device\n", argv[0]);
        return EXIT_FAILURE;
    }

    devicePath = argv[1];

    if ((instrumentFd = openSerialFd(devicePath)) < 0)
    {
        fprintf(stderr, "%s: Failed to open serial device: %s!\n",
                programName,
                strerror(errno));
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
        .write = instrumentWrite,
        .sample = instrumentSample,
        .sampleBuffer = &callbackSample
    };

    if ((err = RBRGen3_open(
             &instrument,
             &callbacks,
             INSTRUMENT_COMMAND_TIMEOUT_MSEC,
             (void *) &instrumentFd)) != RBRGEN3_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to establish instrument connection: %s!\n",
                programName,
                RBRGen3Error_name(err));
        status = EXIT_FAILURE;
        goto fileCleanup;
    }

    RBRInstrumentGen3Link link;
    RBRInstrumentGen3_getLink(instrument, &link);

    switch (link)
    {
    case RBRINSTRUMENTGEN3_LINK_USB:
        RBRInstrumentGen3_setUSBStreamingState(instrument, true);
        break;
    case RBRINSTRUMENTGEN3_LINK_SERIAL:
    case RBRINSTRUMENTGEN3_LINK_WIFI:
        RBRInstrumentGen3_setSerialStreamingState(instrument, true);
        break;
    default:
        fprintf(stderr,
                "%s: I don't know how I'm connected to the instrument, so I"
                " can't enable streaming. Giving up.\n",
                programName);
        goto instrumentCleanup;
    }

    RBRInstrumentGen3Deployment deployment;
    RBRInstrumentGen3_getDeployment(instrument, &deployment);
    if (deployment.status != RBRINSTRUMENTGEN3_STATUS_LOGGING)
    {
        printf("%s: Instrument is %s, not logging. I'm going to start it.\n",
               programName,
               RBRInstrumentGen3DeploymentStatus_name(deployment.status));

        if ((err = instrumentStart(instrument)) != RBRGEN3_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Failed to start instrument: %s!\n",
                    programName,
                    RBRGen3Error_name(err));
            status = EXIT_FAILURE;
            goto instrumentCleanup;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "%s: Failed to initialize SDL!\n", programName);
        goto instrumentCleanup;
    }

    window = SDL_CreateWindow(
        "POSIX Streaming Example",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL)
    {
        fprintf(stderr, "%s: Failed to initialize SDL window!\n", programName);
        goto instrumentCleanup;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        fprintf(stderr,
                "%s: Failed to initialize SDL window/renderer!\n",
                programName);
        goto instrumentCleanup;
    }

    SDL_bool done = SDL_FALSE;

    while (!done)
    {
        if ((err = RBRInstrumentGen3_readSample(instrument)) != RBRGEN3_SUCCESS)
        {
            fprintf(stderr,
                    "%s: Error: %s\n",
                    programName,
                    RBRGen3Error_name(err));
        }

        /* We've received new samples; recalculate all the points. */
        if (sampleCount >= 2 &&
            (samplePointCount < sampleCount || sampleCount == SAMPLE_SIZE))
        {
            recalculatePoints();
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for (int channel = 0; channel < samplePointChannelCount; channel++)
        {
            SDL_SetRenderDrawColor(renderer,
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][0],
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][1],
                                   PLOT_COLORS[channel % PLOT_COLORS_LEN][2],
                                   SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLines(renderer,
                                samplePoints[channel],
                                samplePointCount);
        }
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                done = SDL_TRUE;
            }
        }
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }
    if (window)
    {
        SDL_DestroyWindow(window);
    }

instrumentCleanup:
    RBRGen3_close(instrument);
fileCleanup:
    close(instrumentFd);

    return status;
}
