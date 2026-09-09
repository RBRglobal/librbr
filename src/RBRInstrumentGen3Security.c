/**
 * \file RBRInstrumentGen3Security.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"

RBRGen3Error RBRInstrumentGen3_permit(RBRGen3 *instrument,
                                        const char *command)
{
    const char *permitCommand;
    if (instrument->generation == RBRGEN3_LOGGER2)
    {
        permitCommand = "permit = %s";
    }
    else
    {
        permitCommand = "permit command = %s";
    }
    return RBRGen3_converse(instrument, permitCommand, command);
}

RBRGen3Error RBRInstrumentGen3_getPrompt(RBRGen3 *instrument,
                                           bool *prompt)
{
    return RBRGen3_getBool(instrument,
                                 "prompt",
                                 "state",
                                 prompt);
}

RBRGen3Error RBRInstrumentGen3_setPrompt(RBRGen3 *instrument,
                                           bool prompt)
{
    return RBRGen3_converse(instrument,
                                  "prompt state = %s",
                                  prompt ? "on" : "off");
}

RBRGen3Error RBRInstrumentGen3_getConfirmation(RBRGen3 *instrument,
                                                 bool *confirmation)
{
    return RBRGen3_getBool(instrument,
                                 "confirmation",
                                 "state",
                                 confirmation);
}

RBRGen3Error RBRInstrumentGen3_setConfirmation(RBRGen3 *instrument,
                                                 bool confirmation)
{
    if (confirmation)
    {
        return RBRGen3_converse(instrument, "confirmation state = on");
    }
    else
    {
        return RBRGen3_sendCommand(instrument,
                                         "confirmation state = off");
    }
}

RBRGen3Error RBRInstrumentGen3_reboot(RBRGen3 *instrument,
                                        int32_t delay)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "reboot"));
    RBR_TRY(RBRGen3_sendCommand(instrument, "reboot %" PRId32, delay));

    instrument->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    return RBRGEN3_SUCCESS;
}
