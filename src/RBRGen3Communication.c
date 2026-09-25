/**
 * \file RBRGen3Communication.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for strtol. */
#include <stdlib.h>
/* Required for strcmp. */
#include <string.h>

#include "RBRGen3.h"
#include "RBRGen3Internal.h"
#include "RBRGen3Communication.h"

const char *RBRGen3Link_name(RBRGen3Link link)
{
    switch (link) {
    case RBRGEN3_LINK_USB:
        return "usb";
    case RBRGEN3_LINK_SERIAL:
        return "serial";
    case RBRGEN3_LINK_WIFI:
        return "wifi";
    case RBRGEN3_LINK_COUNT:
        return "link count";
    case RBRGEN3_UNKNOWN_LINK:
    default:
        return "unknown link";
    }
}

RBRGen3Error RBRGen3_getLink(RBRGen3 *conn, RBRGen3Link *link)
{
    *link = RBRGEN3_UNKNOWN_LINK;

    RBR_TRY(RBRGen3_converse(conn, "link"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "link") == 0 || strcmp(parameter.key, "type") == 0) {
            for (int i = 0; i < RBRGEN3_LINK_COUNT; i++) {
                if (strcmp(RBRGen3Link_name(i), parameter.value) == 0) {
                    *link = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

const char *RBRGen3SerialBaudRate_name(RBRGen3SerialBaudRate baud)
{
    switch (baud) {
    case RBRGEN3_SERIAL_BAUD_NONE:
        return "none";
    case RBRGEN3_SERIAL_BAUD_300:
        return "300";
    case RBRGEN3_SERIAL_BAUD_600:
        return "600";
    case RBRGEN3_SERIAL_BAUD_1200:
        return "1200";
    case RBRGEN3_SERIAL_BAUD_2400:
        return "2400";
    case RBRGEN3_SERIAL_BAUD_4800:
        return "4800";
    case RBRGEN3_SERIAL_BAUD_9600:
        return "9600";
    case RBRGEN3_SERIAL_BAUD_19200:
        return "19200";
    case RBRGEN3_SERIAL_BAUD_28800:
        return "28800";
    case RBRGEN3_SERIAL_BAUD_38400:
        return "38400";
    case RBRGEN3_SERIAL_BAUD_57600:
        return "57600";
    case RBRGEN3_SERIAL_BAUD_115200:
        return "115200";
    case RBRGEN3_SERIAL_BAUD_230400:
        return "230400";
    case RBRGEN3_SERIAL_BAUD_460800:
        return "460800";
    case RBRGEN3_SERIAL_BAUD_921600:
        return "921600";
    default:
        return "unknown baud";
    }
}

const char *RBRGen3SerialMode_name(RBRGen3SerialMode mode)
{
    switch (mode) {
    case RBRGEN3_SERIAL_MODE_NONE:
        return "none";
    case RBRGEN3_SERIAL_MODE_RS232:
        return "rs232";
    case RBRGEN3_SERIAL_MODE_RS485F:
        return "rs485f";
    case RBRGEN3_SERIAL_MODE_RS485H:
        return "rs485h";
    case RBRGEN3_SERIAL_MODE_UART:
        return "uart";
    case RBRGEN3_SERIAL_MODE_UART_IDLE_LOW:
        return "uart_idlelow";
    default:
        return "unknown serial mode";
    }
}

RBRGen3Error RBRGen3_getSerial(RBRGen3 *conn, RBRGen3Serial *serial)
{
    memset(serial, 0, sizeof(RBRGen3Serial));

    RBRGen3SerialBaudRate *availableBaudRates =
        (RBRGen3SerialBaudRate *) &serial->availableBaudRates;
    RBRGen3SerialMode *availableModes = (RBRGen3SerialMode *) &serial->availableModes;

    if (conn->generation == RBRCOMMON_LOGGER2) {
        *availableBaudRates = RBRGEN3_SERIAL_BAUD_1200 | RBRGEN3_SERIAL_BAUD_2400 |
                              RBRGEN3_SERIAL_BAUD_4800 | RBRGEN3_SERIAL_BAUD_9600 |
                              RBRGEN3_SERIAL_BAUD_19200 | RBRGEN3_SERIAL_BAUD_115200;
        *availableModes = RBRGEN3_SERIAL_MODE_RS232 | RBRGEN3_SERIAL_MODE_RS485F |
                          RBRGEN3_SERIAL_MODE_UART | RBRGEN3_SERIAL_MODE_UART_IDLE_LOW;

        RBR_TRY(RBRGen3_converse(conn, "serial"));
    } else {
        *availableBaudRates = RBRGEN3_SERIAL_BAUD_NONE;
        *availableModes = RBRGEN3_SERIAL_MODE_NONE;

        RBR_TRY(RBRGen3_converse(conn, "serial all"));
    }

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "baudrate") == 0) {
            for (int i = RBRGEN3_SERIAL_BAUD_NONE + 1; i <= RBRGEN3_SERIAL_BAUD_MAX; i <<= 1) {
                if (strcmp(RBRGen3SerialBaudRate_name(i), parameter.value) == 0) {
                    serial->baudRate = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "mode") == 0) {
            for (int i = RBRGEN3_SERIAL_MODE_NONE + 1; i <= RBRGEN3_SERIAL_MODE_MAX; i <<= 1) {
                if (strcmp(RBRGen3SerialMode_name(i), parameter.value) == 0) {
                    serial->mode = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "availablebaudrates") == 0) {
            char *nextValue;
            do {
                if ((nextValue = strstr(parameter.value, "|")) != NULL) {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRGEN3_SERIAL_BAUD_NONE + 1; i <= RBRGEN3_SERIAL_BAUD_MAX; i <<= 1) {
                    if (strcmp(RBRGen3SerialBaudRate_name(i), parameter.value) == 0) {
                        *availableBaudRates |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        } else if (strcmp(parameter.key, "availablemodes") == 0) {
            char *nextValue;
            do {
                if ((nextValue = strstr(parameter.value, "|")) != NULL) {
                    *nextValue = '\0';
                    nextValue++;
                }

                for (int i = RBRGEN3_SERIAL_MODE_NONE + 1; i <= RBRGEN3_SERIAL_MODE_MAX; i <<= 1) {
                    if (strcmp(RBRGen3SerialMode_name(i), parameter.value) == 0) {
                        *availableModes |= i;
                    }
                }

                parameter.value = nextValue;
            } while (nextValue != NULL);
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setSerial(RBRGen3 *conn, const RBRGen3Serial *serial)
{
    if (serial->baudRate < 0 || serial->baudRate > RBRGEN3_SERIAL_BAUD_MAX || serial->mode < 0 ||
        serial->mode > RBRGEN3_SERIAL_MODE_MAX) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    return RBRGen3_converse(conn,
                            "serial baudrate = %s, mode = %s",
                            RBRGen3SerialBaudRate_name(serial->baudRate),
                            RBRGen3SerialMode_name(serial->mode));
}

RBRGen3Error RBRGen3_sleep(RBRGen3 *conn)
{
    RBR_TRY(RBRGen3_sendCommand(conn, "sleep"));

    conn->lastActivityTime = RBRGEN3_NO_ACTIVITY;
    return RBRGEN3_SUCCESS;
}

const char *RBRGen3WiFiState_name(RBRGen3WiFiState state)
{
    switch (state) {
    case RBRGEN3_WIFI_NA:
        return "n/a";
    case RBRGEN3_WIFI_ON:
        return "on";
    case RBRGEN3_WIFI_OFF:
        return "off";
    case RBRGEN3_WIFI_COUNT:
        return "state count";
    case RBRGEN3_UNKNOWN_WIFI:
    default:
        return "unknown state";
    }
}

RBRGen3Error RBRGen3_getWiFi(RBRGen3 *conn, RBRGen3WiFi *wifi)
{
    memset(wifi, 0, sizeof(RBRGen3WiFi));

    RBRGen3WiFiState *state = (RBRGen3WiFiState *) &wifi->state;
    *state = RBRGEN3_UNKNOWN_WIFI;

    RBR_TRY(RBRGen3_converse(conn, "wifi"));

    char *command = NULL;
    RBRGen3ResponseParameter parameter;
    while (true) {
        RBRGen3_parseResponse(conn, &command, &parameter);

        if (parameter.key == NULL || parameter.value == NULL) {
            break;
        } else if (strcmp(parameter.key, "enabled") == 0) {
            wifi->enabled = (strcmp(parameter.value, "true") == 0);
        } else if (strcmp(parameter.key, "state") == 0) {
            for (int i = RBRGEN3_WIFI_NA; i < RBRGEN3_WIFI_COUNT; i++) {
                if (strcmp(RBRGen3WiFiState_name(i), parameter.value) == 0) {
                    *state = i;
                    break;
                }
            }
        } else if (strcmp(parameter.key, "timeout") == 0) {
            wifi->powerTimeout = strtol(parameter.value, NULL, 10) * 1000;
        } else if (strcmp(parameter.key, "commandtimeout") == 0) {
            wifi->commandTimeout = strtol(parameter.value, NULL, 10) * 1000;
        } else if (strcmp(parameter.key, "baudrate") == 0) {
            for (int i = RBRGEN3_SERIAL_BAUD_NONE + 1; i <= RBRGEN3_SERIAL_BAUD_MAX; i <<= 1) {
                if (strcmp(RBRGen3SerialBaudRate_name(i), parameter.value) == 0) {
                    *(RBRGen3SerialBaudRate *) &wifi->baudRate = i;
                    break;
                }
            }
        }
    }

    return RBRGEN3_SUCCESS;
}

RBRGen3Error RBRGen3_setWiFi(RBRGen3 *conn, const RBRGen3WiFi *wifi)
{
    if (wifi->powerTimeout < 5000 || wifi->powerTimeout > 600000 ||
        wifi->powerTimeout % 1000 != 0 || wifi->commandTimeout < 5000 ||
        wifi->commandTimeout > 600000 || wifi->commandTimeout % 1000 != 0) {
        return RBRGEN3_INVALID_PARAMETER_VALUE;
    }

    if (conn->generation == RBRCOMMON_LOGGER2) {
        return RBRGen3_converse(conn,
                                "wifi timeout = %d, commandtimeout = %d",
                                wifi->powerTimeout / 1000,
                                wifi->commandTimeout / 1000);
    } else {
        return RBRGen3_converse(conn,
                                "wifi enabled = %s, timeout = %d, commandtimeout = %d",
                                wifi->enabled ? "true" : "false",
                                wifi->powerTimeout / 1000,
                                wifi->commandTimeout / 1000);
    }
}
