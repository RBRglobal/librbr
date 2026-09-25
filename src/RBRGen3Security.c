/**
 * \file RBRGen3Security.c
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
#include "RBRGen3Security.h"

RBRGen3Error RBRGen3_permit(RBRGen3 *conn, const char *command)
{
    const char *permitCommand;
    if (conn->generation == RBRCOMMON_LOGGER2) {
        permitCommand = "permit = %s";
    } else {
        permitCommand = "permit command = %s";
    }
    return RBRGen3_converse(conn, permitCommand, command);
}

RBRGen3Error RBRGen3_getPrompt(RBRGen3 *conn, bool *prompt)
{
    return RBRGen3_getBool(conn, "prompt", "state", prompt);
}

RBRGen3Error RBRGen3_setPrompt(RBRGen3 *conn, bool prompt)
{
    return RBRGen3_converse(conn, "prompt state = %s", prompt ? "on" : "off");
}

RBRGen3Error RBRGen3_getConfirmation(RBRGen3 *conn, bool *confirmation)
{
    return RBRGen3_getBool(conn, "confirmation", "state", confirmation);
}

RBRGen3Error RBRGen3_setConfirmation(RBRGen3 *conn, bool confirmation)
{
    if (confirmation) {
        return RBRGen3_converse(conn, "confirmation state = on");
    } else {
        return RBRGen3_sendCommand(conn, "confirmation state = off");
    }
}

RBRGen3Error RBRGen3_reboot(RBRGen3 *conn, int32_t delay)
{
    RBR_TRY(RBRGen3_permit(conn, "reboot"));
    RBR_TRY(RBRGen3_sendCommand(conn, "reboot %" PRId32, delay));

    conn->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    return RBRGEN3_SUCCESS;
}
