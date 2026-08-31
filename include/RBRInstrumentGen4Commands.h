/**
 * \file RBRInstrumentGen4Commands.h
 *
 * \brief Entry point for instrument command declarations.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4COMMANDS_H
#define LIBRBR_RBRINSTRUMENTGEN4COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

/** The order is important. 
 * Based on dependency, it has to be "communication", "configuration", "schedule";
 * And "Memory", "Deployment. "*/
#include "RBRInstrumentGen4Communication.h"
#include "RBRInstrumentGen4Configuration.h"
#include "RBRInstrumentGen4Streaming.h"
#include "RBRInstrumentGen4Memory.h"
#include "RBRInstrumentGen4Deployment.h"
#include "RBRInstrumentGen4Instrument.h"
#include "RBRInstrumentGen4Realtime.h"

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTCOMMANDS_H */

