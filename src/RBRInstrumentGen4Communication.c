/**
 * \file RBRInstrumentGen4Communication.c
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
#include "RBRInstrumentGen4Communication.h"

const char *RBRInstrumentGen4Link_name(RBRInstrumentGen4Link link)
{
    switch (link)
    {
    case RBRINSTRUMENTGEN4_LINK_USB:
        return "usb";
    case RBRINSTRUMENTGEN4_LINK_SERIAL:
        return "serial";
    case RBRINSTRUMENTGEN4_LINK_WIFI:
        return "wifi";
    case RBRINSTRUMENTGEN4_LINK_COUNT:
        return "link count";
    case RBRINSTRUMENTGEN4_UNKNOWN_LINK:
    default:
        return "unknown link";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getLink(RBRInstrumentGen4 *instrument,
                                                 RBRInstrumentGen4Link *link)
{
    *link = RBRINSTRUMENTGEN4_UNKNOWN_LINK;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "link"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "link") == 0 || strcmp(parameter.key, "type") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_LINK_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4Link_name(i),
                           parameter.value) == 0)
                {
                    *link = i;
                    break;
                }
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4SerialBaudRate_name(RBRInstrumentGen4SerialBaudRate baud)
{
    switch (baud)
    {
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_300:
        return "300";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_600:
        return "600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_1200:
        return "1200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_2400:
        return "2400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_4800:
        return "4800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_9600:
        return "9600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_19200:
        return "19200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_28800:
        return "28800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_38400:
        return "38400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_57600:
        return "57600";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_115200:
        return "115200";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_230400:
        return "230400";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_460800:
        return "460800";
    case RBRINSTRUMENTGEN4_SERIAL_BAUD_921600:
        return "921600";
    default:
        return "unknown baud";
    }
}

const char *RBRInstrumentGen4SerialMode_name(RBRInstrumentGen4SerialMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_SERIAL_MODE_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS232:
        return "rs232";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_RS485H:
        return "rs485h";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_UART:
        return "uart";
    case RBRINSTRUMENTGEN4_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    default:
        return "unknown serial mode";
    }
}

const char *RBRInstrumentGen4Aux1ActiveState_name(
    RBRInstrumentGen4Aux1ActiveState aux1_active)
{
    switch (aux1_active)
    {
    case RBRINSTRUMENTGEN4_AUX1ACTIVE_HIGH:
        return "high";
    case RBRINSTRUMENTGEN4_AUX1ACTIVE_LOW:
        return "low";
    case RBRINSTRUMENTGEN4_AUX1ACTIVE_COUNT:
        return "active output level count";
    case RBRINSTRUMENTGEN4_UNKNOWN_AUX1ACTIVE:
    default:
        return "unknown active output level";
    }
}

const char *RBRInstrumentGen4Aux1SleepState_name(RBRInstrumentGen4Aux1SleepState aux1_sleep)
{

    switch (aux1_sleep)
    {
    case RBRINSTRUMENTGEN4_AUX1SLEEP_TRISTATE:
        return "tristate";
    case RBRINSTRUMENTGEN4_AUX1SLEEP_HIGH:
        return "high";
    case RBRINSTRUMENTGEN4_AUX1SLEEP_LOW:
        return "low";
    case RBRINSTRUMENTGEN4_AUX1SLEEP_COUNT:
        return "sleep output level count";
    case RBRINSTRUMENTGEN4_UNKNOWN_AUX1SLEEP:
    default:
        return "unknown aux1_sleep level";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getAux1(RBRInstrumentGen4 *instrument,
                                                 RBRInstrumentGen4Aux1 *aux1)
{
    memset(aux1, 0, sizeof(RBRInstrumentGen4Aux1));
    aux1->aux1_active = RBRINSTRUMENTGEN4_UNKNOWN_AUX1ACTIVE;
    aux1->aux1_sleep = RBRINSTRUMENTGEN4_UNKNOWN_AUX1SLEEP;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "serial aux1_all"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        if (strcmp(parameter.key, "aux1_state") == 0)
        {
            aux1->aux1_state = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "aux1_setup") == 0)
        {
            aux1->aux1_setup = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_hold") == 0)
        {
            aux1->aux1_hold = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "aux1_active") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_AUX1ACTIVE_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4Aux1ActiveState_name(i),
                           parameter.value) == 0)
                {
                    aux1->aux1_active = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "aux1_sleep") == 0)
        {
            for (int i = 0; i < RBRINSTRUMENTGEN4_AUX1SLEEP_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4Aux1SleepState_name(i),
                           parameter.value) == 0)
                {
                    aux1->aux1_sleep = i;
                    break;
                }
            }
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setAux1(RBRInstrumentGen4 *instrument,
                                                 RBRInstrumentGen4Aux1 *aux1)
{
    const char *enabledValue;
    enabledValue = aux1->aux1_state ? "on" : "off";

    return RBRInstrumentGen4_converse(
        instrument,
        "serial aux1_state = %s, aux1_setup = %" PRIi32 ", "
        "aux1_hold = %" PRIi32 ", aux1_active = %s, aux1_sleep = %s",
        enabledValue,
        aux1->aux1_setup,
        aux1->aux1_hold,
        RBRInstrumentGen4Aux1ActiveState_name(aux1->aux1_active),
        RBRInstrumentGen4Aux1SleepState_name(aux1->aux1_sleep));
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSerial(RBRInstrumentGen4 *instrument,
                                                   RBRInstrumentGen4Serial *serial)
{
    memset(serial, 0, sizeof(RBRInstrumentGen4Serial));

    RBRInstrumentGen4SerialBaudRate *availableBaudRates =
        (RBRInstrumentGen4SerialBaudRate *) &serial->availableBaudRates;
    RBRInstrumentGen4SerialMode *availableModes =
        (RBRInstrumentGen4SerialMode *) &serial->availableModes;

    *availableBaudRates = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE;
    *availableModes = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "serial all"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    serial->baudRate = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_MODE_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialMode_name(i),
                           parameter.value) == 0)
                {
                    serial->mode = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "availablebaudrates") == 0)
        {
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                     i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                               parameter.value) == 0)
                    {
                        *availableBaudRates |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
        else if (strcmp(parameter.key, "availablemodes") == 0)
        {
            char *nextValue;
            do
            {
                if ((nextValue = strstr(parameter.value, "|")) != NULL)
                {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRINSTRUMENTGEN4_SERIAL_MODE_NONE + 1;
                     i <= RBRINSTRUMENTGEN4_SERIAL_MODE_MAX;
                     i <<= 1)
                {
                    if (strcmp(RBRInstrumentGen4SerialMode_name(i),
                               parameter.value) == 0)
                    {
                        *availableModes |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSerial(RBRInstrumentGen4 *instrument,
                                                   const RBRInstrumentGen4Serial *serial)
{
    if (serial->baudRate < 0 || serial->baudRate > RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX || serial->mode < 0 || serial->mode > RBRINSTRUMENTGEN4_SERIAL_MODE_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "serial baudrate = %s, mode = %s",
        RBRInstrumentGen4SerialBaudRate_name(serial->baudRate),
        RBRInstrumentGen4SerialMode_name(serial->mode));
}

RBRInstrumentGen4Error RBRInstrumentGen4_sleep(RBRInstrumentGen4 *instrument)
{
    RBR_TRY(RBRInstrumentGen4_sendCommand(instrument, "sleep"));
    instrument->lastActivityTime = RBRINSTRUMENTGEN4_NO_ACTIVITY;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4WiFiState_name(RBRInstrumentGen4WiFiState state)
{
    switch (state)
    {
    case RBRINSTRUMENTGEN4_WIFI_NA:
        return "n/a";
    case RBRINSTRUMENTGEN4_WIFI_ON:
        return "on";
    case RBRINSTRUMENTGEN4_WIFI_OFF:
        return "off";
    case RBRINSTRUMENTGEN4_WIFI_COUNT:
        return "state count";
    case RBRINSTRUMENTGEN4_UNKNOWN_WIFI:
    default:
        return "unknown state";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getWiFi(RBRInstrumentGen4 *instrument,
                                         RBRInstrumentGen4WiFi *wifi)
{
     memset(wifi, 0, sizeof(RBRInstrumentGen4WiFi));

    RBRInstrumentGen4WiFiState *state = (RBRInstrumentGen4WiFiState *)&wifi->state;
    *state = RBRINSTRUMENTGEN4_UNKNOWN_WIFI;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "wifi"));

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "enabled") == 0)
        {
            wifi->enabled = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "state") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_WIFI_NA;
                 i < RBRINSTRUMENTGEN4_WIFI_COUNT;
                 i++)
            {
                if (strcmp(RBRInstrumentGen4WiFiState_name(i),
                           parameter.value) == 0)
                {
                    *state = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "timeout") == 0)
        {
            wifi->timeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "commandtimeout") == 0)
        {
            wifi->commandTimeout = strtol(parameter.value, NULL, 10) * 1000;
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            for (int i = RBRINSTRUMENTGEN4_SERIAL_BAUD_NONE + 1;
                 i <= RBRINSTRUMENTGEN4_SERIAL_BAUD_MAX;
                 i <<= 1)
            {
                if (strcmp(RBRInstrumentGen4SerialBaudRate_name(i),
                           parameter.value) == 0)
                {
                    *(RBRInstrumentGen4SerialBaudRate *) &wifi->baudRate = i;
                    break;
                }
            }
        }
    }
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setWiFi(RBRInstrumentGen4 *instrument,
                                         const RBRInstrumentGen4WiFi *wifi)
{
    if (wifi->timeout < 5000
        || wifi->timeout > 600000
        || wifi->timeout % 1000 != 0
        || wifi->commandTimeout < 5000
        || wifi->commandTimeout > 600000
        || wifi->commandTimeout % 1000 != 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(
            instrument,
            "wifi enabled = %s, timeout = %d, commandtimeout = %d",
            wifi->enabled ? "true" : "false",
            wifi->timeout / 1000,
            wifi->commandTimeout / 1000);
}