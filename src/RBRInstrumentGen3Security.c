/**
 * \file RBRInstrumentSecurity.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

RBRInstrumentError RBRInstrument_permit(RBRInstrument *instrument,
                                        const char *command)
{
    const char *permitCommand;
    if (instrument->generation == RBRINSTRUMENT_LOGGER2)
    {
        permitCommand = "permit = %s";
    }
    else
    {
        permitCommand = "permit command = %s";
    }
    return RBRInstrument_converse(instrument, permitCommand, command);
}

RBRInstrumentError RBRInstrument_getPrompt(RBRInstrument *instrument,
                                           bool *prompt)
{
    return RBRInstrument_getBool(instrument,
                                 "prompt",
                                 "state",
                                 prompt);
}

RBRInstrumentError RBRInstrument_setPrompt(RBRInstrument *instrument,
                                           bool prompt)
{
    return RBRInstrument_converse(instrument,
                                  "prompt state = %s",
                                  prompt ? "on" : "off");
}

RBRInstrumentError RBRInstrument_getConfirmation(RBRInstrument *instrument,
                                                 bool *confirmation)
{
    return RBRInstrument_getBool(instrument,
                                 "confirmation",
                                 "state",
                                 confirmation);
}

RBRInstrumentError RBRInstrument_setConfirmation(RBRInstrument *instrument,
                                                 bool confirmation)
{
    if (confirmation)
    {
        return RBRInstrument_converse(instrument, "confirmation state = on");
    }
    else
    {
        return RBRInstrument_sendCommand(instrument,
                                         "confirmation state = off");
    }
}

RBRInstrumentError RBRInstrument_reboot(RBRInstrument *instrument,
                                        int32_t delay)
{
    RBR_TRY(RBRInstrument_permit(instrument, "reboot"));
    RBR_TRY(RBRInstrument_sendCommand(instrument, "reboot %" PRId32, delay));

    instrument->lastActivityTime = RBRINSTRUMENT_NO_ACTIVITY;
    return RBRINSTRUMENT_SUCCESS;
}
