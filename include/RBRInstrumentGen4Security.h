/**
 * \file RBRInstrumentGen4Security.h
 *
 * \brief Instrument commands and structures pertaining to command security and
 * interaction.
 *
 * \see https://docs.rbr-global.com/L3commandreference/commands/security-and-interaction
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4SECURITY_H
#define LIBRBR_RBRINSTRUMENTGEN4SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"

/**
 * \brief Permits a protected command to be executed.
 *
 * Permits a protected command to be executed immediately after this one;
 * receipt of anything else removes the permission again. Any other constraints
 * on executing a particular command will still apply. It is not an error to
 * `permit` a command which does not need it, merely unnecessary.
 *
 * \param [in] instrument the instrument connection
 * \param [in] command the command to permit
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the command has been permitted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the command can't be permitted
 * \see https://docs.rbr-global.com/L3commandreference/commands/security-and-interaction/permit
 */
RBRInstrumentGen4Error RBRInstrumentGen4_permit(RBRInstrumentGen4 *instrument,
                                        const char *command);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4SECURITY_H */
