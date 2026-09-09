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

#include "RBRInstrumentGen3.h"
#include "RBRInstrumentGen3Internal.h"

RBRInstrumentGen3Error RBRInstrumentGen3_permit(RBRInstrumentGen3 *instrument,
                                        const char *command)
{
    const char *permitCommand;
    if (instrument->generation == RBRINSTRUMENTGEN3_LOGGER2)
    {
        permitCommand = "permit = %s";
    }
    else
    {
        permitCommand = "permit command = %s";
    }
    return RBRInstrumentGen3_converse(instrument, permitCommand, command);
}

RBRInstrumentGen3Error RBRInstrumentGen3_getPrompt(RBRInstrumentGen3 *instrument,
                                           bool *prompt)
{
    return RBRInstrumentGen3_getBool(instrument,
                                 "prompt",
                                 "state",
                                 prompt);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setPrompt(RBRInstrumentGen3 *instrument,
                                           bool prompt)
{
    return RBRInstrumentGen3_converse(instrument,
                                  "prompt state = %s",
                                  prompt ? "on" : "off");
}

RBRInstrumentGen3Error RBRInstrumentGen3_getConfirmation(RBRInstrumentGen3 *instrument,
                                                 bool *confirmation)
{
    return RBRInstrumentGen3_getBool(instrument,
                                 "confirmation",
                                 "state",
                                 confirmation);
}

RBRInstrumentGen3Error RBRInstrumentGen3_setConfirmation(RBRInstrumentGen3 *instrument,
                                                 bool confirmation)
{
    if (confirmation)
    {
        return RBRInstrumentGen3_converse(instrument, "confirmation state = on");
    }
    else
    {
        return RBRInstrumentGen3_sendCommand(instrument,
                                         "confirmation state = off");
    }
}

RBRInstrumentGen3Error RBRInstrumentGen3_reboot(RBRInstrumentGen3 *instrument,
                                        int32_t delay)
{
    RBR_TRY(RBRInstrumentGen3_permit(instrument, "reboot"));
    RBR_TRY(RBRInstrumentGen3_sendCommand(instrument, "reboot %" PRId32, delay));

    instrument->lastActivityTime = RBRINSTRUMENTGEN3_NO_ACTIVITY;
    return RBRINSTRUMENTGEN3_SUCCESS;
}
