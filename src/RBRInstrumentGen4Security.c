/**
 * \file RBRInstrumentGen4Security.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Security.h"

RBRInstrumentGen4Error RBRInstrumentGen4_permit(RBRInstrumentGen4 *instrument,
                                        const char *command)
{
    const char *permitCommand;
    permitCommand = "permit command=%s";
    return RBRInstrumentGen4_converse(instrument, permitCommand, command);
}
