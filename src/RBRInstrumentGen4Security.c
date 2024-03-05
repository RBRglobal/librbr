/**
 * \file RBRInstrumentGen4Security.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strcmp. */
#include <string.h>

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Security.h"

RBRInstrumentGen4Error RBRInstrumentGen4_permit(RBRInstrumentGen4 *instrument,
                                        const char *command)
{
    const char *permitCommand;
    permitCommand = "permit command = %s";
    return RBRInstrumentGen4_converse(instrument, permitCommand, command);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPrompt(RBRInstrumentGen4 *instrument,
                                           bool *prompt)
{
    return RBRInstrumentGen4_getBool(instrument,
                                 "prompt",
                                 "state",
                                 prompt);
}

RBRInstrumentGen4Error RBRInstrumentGen4_setPrompt(RBRInstrumentGen4 *instrument,
                                           const bool prompt)
{
    return RBRInstrumentGen4_converse(instrument,
                                  "prompt state = %s",
                                  prompt ? "on" : "off");
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfirmation(RBRInstrumentGen4 *instrument,
                                                 bool *confirmation)
{
    return RBRInstrumentGen4_getBool(instrument,
                                 "confirmation",
                                 "state",
                                 confirmation);
}

RBRInstrumentGen4Error RBRInstrumentGen4_setConfirmation(RBRInstrumentGen4 *instrument,
                                                 const bool confirmation)
{
    if (confirmation)
    {
        return RBRInstrumentGen4_converse(instrument, "confirmation state = on");
    }
    else
    {
        //because when turned off, there will be nothing in return.
        return RBRInstrumentGen4_sendCommand(instrument,
                                         "confirmation state = off");
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_reboot(RBRInstrumentGen4 *instrument,
                                        const int32_t delay)
{
    RBR_TRY(RBRInstrumentGen4_permit(instrument, "reboot"));
    RBR_TRY(RBRInstrumentGen4_sendCommand(instrument, "reboot %" PRId32, delay));

    instrument->lastActivityTime = RBRINSTRUMENTGEN4_NO_ACTIVITY;
    return RBRINSTRUMENTGEN4_SUCCESS;
}
