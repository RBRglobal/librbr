/**
 * \file RBRGen4Configuration.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

/* Required for PRId32. */
#include <inttypes.h>
/* Required for NAN. */
#include <math.h>
/* Required for snprintf. */
#include <stdio.h>
/* Required for memset, strcmp. */
#include <stdlib.h>
#include <string.h>

#include "RBRGen4.h"
#include "RBRGen4Internal.h"
#include "RBRGen4Configuration.h"

RBRGen4Error RBRGen4_getNode(
    RBRGen4 *instrument,
    RBRGen4Node *node)
{
    /* The label selects the node to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(node->label)];
    snprintf(label, sizeof(label), "%s", node->label);

    memset(node, 0, sizeof(RBRGen4Node));

    RBR_TRY(RBRGen4_converse(instrument, "node %s", label));

    snprintf(node->label, sizeof(node->label), "%s", label);

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "pcba") == 0)
        {
            snprintf(node->pcba,
                     sizeof(node->pcba),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "portlist") == 0)
        {
            /* A node with no ports reports `none`, not an empty list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && node->portCount < RBRGEN4_PORT_COUNT_MAX)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(node->portList[node->portCount],
                         sizeof(node->portList[node->portCount]),
                         "%s",
                         value);
                node->portCount++;

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf(node->fwVersion,
                     sizeof(node->fwVersion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "semver") == 0)
        {
            snprintf(node->semver,
                     sizeof(node->semver),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            /* `na` is not a number, so it converts to the zero which stands
             * for it. */
            node->fwType = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "poweruptime") == 0)
        {
            node->powerUpTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "inrushoffsettime") == 0)
        {
            node->inrushOffsetTime = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getNodePool(
    RBRGen4 *instrument,
    RBRGen4NodePool *nodePool)
{
    memset(nodePool, 0, sizeof(RBRGen4NodePool));

    RBR_TRY(RBRGen4_converse(instrument, "node"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            nodePool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no nodes reports `none` to indicate an empty list */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t node = 0;
                 value != NULL && node < RBRGEN4_NODE_COUNT_MAX;
                 node++)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(nodePool->pool[node].label,
                         sizeof(nodePool->pool[node].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4PortClass_name(
    RBRGen4PortClass portClass)
{
    switch (portClass)
    {
    case RBRGEN4_PORT_CLASS_VIRTUAL:
        return "virtual";
    case RBRGEN4_PORT_CLASS_SERIAL:
        return "serial";
    case RBRGEN4_PORT_CLASS_COUNT:
        return "port class count";
    case RBRGEN4_UNKNOWN_PORT_CLASS:
    default:
        return "unknown port class";
    }
}

const char *RBRGen4PortProtocol_name(
    RBRGen4PortProtocol protocol)
{
    switch (protocol)
    {
    case RBRGEN4_PORT_PROTOCOL_PRESSURE:
        return "pressure";
    case RBRGEN4_PORT_PROTOCOL_RBRSERIAL:
        return "rbrserial";
    case RBRGEN4_PORT_PROTOCOL_RBRMODEM:
        return "rbrmodem";
    case RBRGEN4_PORT_PROTOCOL_RBRMULTIDROP:
        return "rbrmultidrop";
    case RBRGEN4_PORT_PROTOCOL_NONE:
    default:
        return "none";
    }
}

/**
 * \brief Find the port protocol a response value names.
 *
 * \param [in] value the response value
 * \return the protocol, or #RBRGEN4_PORT_PROTOCOL_NONE
 */
static RBRGen4PortProtocol RBRGen4PortProtocol_parse(
    const char *value)
{
    for (int i = RBRGEN4_PORT_PROTOCOL_NONE + 1;
         i <= RBRGEN4_PORT_PROTOCOL_MAX;
         i <<= 1)
    {
        if (strcmp(RBRGen4PortProtocol_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_PORT_PROTOCOL_NONE;
}

RBRGen4Error RBRGen4_getPort(
    RBRGen4 *instrument,
    RBRGen4Port *port)
{
    /* The label selects the port to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(port->label)];
    snprintf(label, sizeof(label), "%s", port->label);

    memset(port, 0, sizeof(RBRGen4Port));
    port->portClass = RBRGEN4_UNKNOWN_PORT_CLASS;

    RBR_TRY(RBRGen4_converse(instrument, "port %s", label));

    snprintf(port->label, sizeof(port->label), "%s", label);

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "node") == 0)
        {
            snprintf(port->node,
                     sizeof(port->node),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "class") == 0)
        {
            for (int32_t i = 0; i < RBRGEN4_PORT_CLASS_COUNT; i++)
            {
                if (strcmp(RBRGen4PortClass_name(i),
                           parameter.value) == 0)
                {
                    port->portClass = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "protocol") == 0)
        {
            port->protocol
                = RBRGen4PortProtocol_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "availableprotocols") == 0)
        {
            char *value = parameter.value;
            while (value != NULL)
            {
                char *nextValue = RBRGen4_splitListValue(value);
                port->availableProtocols
                    |= RBRGen4PortProtocol_parse(value);

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "baudrate") == 0)
        {
            port->baudRate = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "devicelist") == 0)
        {
            /* A port with no devices reports `none`, not an empty list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && port->deviceCount < RBRGEN4_DEVICE_COUNT_MAX)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(port->deviceList[port->deviceCount],
                         sizeof(port->deviceList[port->deviceCount]),
                         "%s",
                         value);
                port->deviceCount++;

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "poweruptime") == 0)
        {
            port->powerUpTime = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getPortPool(
    RBRGen4 *instrument,
    RBRGen4PortPool *portPool)
{
    memset(portPool, 0, sizeof(RBRGen4PortPool));

    /* Zero is a real port class, so say the class is unknown until
     * RBRGen4_getPort() reads it. */
    for (int32_t port = 0; port < RBRGEN4_PORT_COUNT_MAX; port++)
    {
        portPool->pool[port].portClass = RBRGEN4_UNKNOWN_PORT_CLASS;
    }

    RBR_TRY(RBRGen4_converse(instrument, "port"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            portPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no ports reports `none`, not an empty list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t port = 0;
                 value != NULL && port < RBRGEN4_PORT_COUNT_MAX;
                 port++)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(portPool->pool[port].label,
                         sizeof(portPool->pool[port].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

const char *RBRGen4DeviceClass_name(
    RBRGen4DeviceClass deviceClass)
{
    switch (deviceClass)
    {
    case RBRGEN4_DEVICE_CLASS_SENSOR:
        return "sensor";
    case RBRGEN4_DEVICE_CLASS_ACTUATOR:
        return "actuator";
    case RBRGEN4_DEVICE_CLASS_EVENTGEN:
        return "eventgen";
    case RBRGEN4_DEVICE_CLASS_MODEM:
        return "modem";
    case RBRGEN4_DEVICE_CLASS_COUNT:
        return "device class count";
    case RBRGEN4_UNKNOWN_DEVICE_CLASS:
    default:
        return "unknown device class";
    }
}

RBRGen4Error RBRGen4_getDevice(
    RBRGen4 *instrument,
    RBRGen4Device *device)
{
    /* The label selects the device to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(device->label)];
    snprintf(label, sizeof(label), "%s", device->label);

    memset(device, 0, sizeof(RBRGen4Device));
    device->deviceClass = RBRGEN4_UNKNOWN_DEVICE_CLASS;

    /* `lock` is deliberately absent from the response to a bare
     * `device <label>`, so name every parameter rather than take the
     * defaults. */
    RBR_TRY(RBRGen4_converse(
        instrument,
        "device %s port class sn pn fwversion fwtype name channellist lock"
        " poweruptime cooldowntime powerdowntime inrushoffsettime",
        label));

    snprintf(device->label, sizeof(device->label), "%s", label);

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "port") == 0)
        {
            snprintf(device->port,
                     sizeof(device->port),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "class") == 0)
        {
            for (int32_t i = 0; i < RBRGEN4_DEVICE_CLASS_COUNT; i++)
            {
                if (strcmp(RBRGen4DeviceClass_name(i),
                           parameter.value) == 0)
                {
                    device->deviceClass = i;
                    break;
                }
            }
        }
        else if (strcmp(parameter.key, "sn") == 0)
        {
            /* `na` is not a number, so it converts to the zero which stands
             * for it, as it does for the firmware type below. */
            device->sn = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "pn") == 0)
        {
            snprintf(device->pn, sizeof(device->pn), "%s", parameter.value);
        }
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf(device->fwVersion,
                     sizeof(device->fwVersion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "fwtype") == 0)
        {
            device->fwType = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "name") == 0)
        {
            snprintf(device->name,
                     sizeof(device->name),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "channellist") == 0)
        {
            /* A device with no channels reports `none`, not an empty list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && device->channelCount < RBRGEN4_CHANNEL_MAX)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(device->channelList[device->channelCount],
                         sizeof(device->channelList[device->channelCount]),
                         "%s",
                         value);
                device->channelCount++;

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "lock") == 0)
        {
            device->lock = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "poweruptime") == 0)
        {
            device->powerUpTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "cooldowntime") == 0)
        {
            device->coolDownTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "powerdowntime") == 0)
        {
            device->powerDownTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "inrushoffsettime") == 0)
        {
            device->inrushOffsetTime = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getDevicePool(
    RBRGen4 *instrument,
    RBRGen4DevicePool *devicePool)
{
    memset(devicePool, 0, sizeof(RBRGen4DevicePool));

    /* Zero is a real device class, so say the class is unknown until
     * RBRGen4_getDevice() reads it. */
    for (int32_t device = 0;
         device < RBRGEN4_DEVICE_COUNT_MAX;
         device++)
    {
        devicePool->pool[device].deviceClass
            = RBRGEN4_UNKNOWN_DEVICE_CLASS;
    }

    RBR_TRY(RBRGen4_converse(instrument, "device"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            devicePool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no devices reports `none`, not an empty
             * list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t device = 0;
                 value != NULL && device < RBRGEN4_DEVICE_COUNT_MAX;
                 device++)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(devicePool->pool[device].label,
                         sizeof(devicePool->pool[device].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_discoverDevices(
    RBRGen4 *instrument,
    RBRGen4DevicePool *devicePool)
{
    memset(devicePool, 0, sizeof(RBRGen4DevicePool));

    /* Zero is a real device class, so say the class is unknown until
     * RBRGen4_getDevice() reads it. */
    for (int32_t device = 0;
         device < RBRGEN4_DEVICE_COUNT_MAX;
         device++)
    {
        devicePool->pool[device].deviceClass
            = RBRGEN4_UNKNOWN_DEVICE_CLASS;
    }

    RBR_TRY(RBRGen4_converse(instrument, "device discover"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "found") == 0)
        {
            /* Discovery finding nothing reports `none`, not an empty list.
             * There is no count to read: the list is the whole answer. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && devicePool->count < RBRGEN4_DEVICE_COUNT_MAX)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(devicePool->pool[devicePool->count].label,
                         sizeof(devicePool->pool[devicePool->count].label),
                         "%s",
                         value);
                devicePool->count++;

                value = nextValue;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_getCalibration(
    RBRGen4 *instrument,
    RBRGen4Calibration *calibration)
{
    /* The label selects the calibration to read, so it has to outlive the
     * reset of the rest of the structure. */
    char label[sizeof(calibration->label)];
    snprintf(label, sizeof(label), "%s", calibration->label);

    memset(calibration, 0, sizeof(RBRGen4Calibration));

    RBR_TRY(RBRGen4_converse(instrument, "calibration %s", label));

    snprintf(calibration->label, sizeof(calibration->label), "%s", label);

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }

        /* A coefficient key is a group letter and an index: `a0`, `m11`. */
        char *end = NULL;
        char group = parameter.key[0];
        int32_t index = strtol(parameter.key + 1, &end, 10);
        bool coefficient =
            (parameter.key[1] != '\0'
             && *end == '\0'
             && index >= 0
             && index < RBRGEN4_CALIBRATION_COEFFICIENT_MAX);

        if (strcmp(parameter.key, "equation") == 0)
        {
            snprintf(calibration->equation,
                     sizeof(calibration->equation),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            calibration->dateTime = strtoll(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "offset") == 0)
        {
            calibration->userOffset = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "slope") == 0)
        {
            calibration->userSlope = strtof(parameter.value, NULL);
        }
        else if (coefficient && group == 'a')
        {
            calibration->a[index] = strtof(parameter.value, NULL);
            if (index >= calibration->aCount)
            {
                calibration->aCount = index + 1;
            }
        }
        else if (coefficient && group == 'b')
        {
            calibration->b[index] = strtof(parameter.value, NULL);
            if (index >= calibration->bCount)
            {
                calibration->bCount = index + 1;
            }
        }
        else if (coefficient && group == 'm')
        {
            /* An unused reference is reported as `none`. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            snprintf(calibration->m[index],
                     sizeof(calibration->m[index]),
                     "%s",
                     parameter.value);
            if (index >= calibration->mCount)
            {
                calibration->mCount = index + 1;
            }
        }
    }

    return RBRGEN4_SUCCESS;
}

/**
 * \brief Append one `<group><index>=<value>` coefficient to a command.
 *
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the coefficient does not
 *                                             fit
 */
static RBRGen4Error RBRGen4Calibration_appendCoefficient(
    char *command,
    int32_t size,
    int32_t *length,
    char group,
    int32_t index,
    float value)
{
    /* Checked before appending: the remaining space is only meaningful while
     * the length is still within the buffer. */
    if (*length < 0 || *length >= size)
    {
        return RBRGEN4_BUFFER_TOO_SMALL;
    }

    int32_t written = snprintf(command + *length,
                               size - *length,
                               " %c%" PRId32 "=%.9g",
                               group,
                               index,
                               (double) value);

    if (written < 0 || *length + written >= size)
    {
        return RBRGEN4_BUFFER_TOO_SMALL;
    }

    *length += written;
    return RBRGEN4_SUCCESS;
}

/**
 * \brief Append one `m<index>=<label>` reference to a command.
 *
 * An empty label is sent as `none`, which is how the instrument reports a
 * reference the equation does not use.
 *
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the reference does not fit
 */
static RBRGen4Error RBRGen4Calibration_appendReference(
    char *command,
    int32_t size,
    int32_t *length,
    int32_t index,
    const char *label)
{
    if (*length < 0 || *length >= size)
    {
        return RBRGEN4_BUFFER_TOO_SMALL;
    }

    int32_t written = snprintf(command + *length,
                               size - *length,
                               " m%" PRId32 "=%s",
                               index,
                               label[0] == '\0' ? "none" : label);

    if (written < 0 || *length + written >= size)
    {
        return RBRGEN4_BUFFER_TOO_SMALL;
    }

    *length += written;
    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setCalibration(
    RBRGen4 *instrument,
    const RBRGen4Calibration *calibration)
{
    if (calibration->aCount < 0
        || calibration->aCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX
        || calibration->bCount < 0
        || calibration->bCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX
        || calibration->mCount < 0
        || calibration->mCount > RBRGEN4_CALIBRATION_COEFFICIENT_MAX)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    char coefficients[RBRGEN4_COMMAND_BUFFER_MAX] = "";
    int32_t length = 0;

    for (int32_t a = 0; a < calibration->aCount; ++a)
    {
        RBR_TRY(RBRGen4Calibration_appendCoefficient(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    'a',
                    a,
                    calibration->a[a]));
    }
    for (int32_t b = 0; b < calibration->bCount; ++b)
    {
        RBR_TRY(RBRGen4Calibration_appendCoefficient(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    'b',
                    b,
                    calibration->b[b]));
    }
    for (int32_t m = 0; m < calibration->mCount; ++m)
    {
        RBR_TRY(RBRGen4Calibration_appendReference(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    m,
                    calibration->m[m]));
    }

    return RBRGen4_converse(
        instrument,
        "calibration %s datetime=%014" PRId64 " offset=%.9g slope=%.9g%s",
        calibration->label,
        calibration->dateTime,
        (double) calibration->userOffset,
        (double) calibration->userSlope,
        coefficients);
}

const char *RBRGen4ScheduleStream_name(
    RBRGen4ScheduleStream stream)
{
    switch (stream)
    {
    case RBRGEN4_SCHEDULE_STREAM_OFF:
        return "off";
    case RBRGEN4_SCHEDULE_STREAM_USB:
        return "usb";
    case RBRGEN4_SCHEDULE_STREAM_SERIAL:
        return "serial";
    case RBRGEN4_SCHEDULE_STREAM_COUNT:
        return "stream destination count";
    case RBRGEN4_UNKNOWN_SCHEDULE_STREAM:
    default:
        return "unknown stream destination";
    }
}

const char *RBRGen4ChannelNature_name(
    RBRGen4ChannelNature nature)
{
    switch (nature)
    {
    case RBRGEN4_CHANNEL_NATURE_SCIENTIFIC:
        return "scientific";
    case RBRGEN4_CHANNEL_NATURE_SYSTEM:
        return "system";
    case RBRGEN4_CHANNEL_NATURE_COUNT:
        return "channel nature count";
    case RBRGEN4_UNKNOWN_CHANNEL_NATURE:
    default:
        return "unknown channel nature";
    }
}

static RBRGen4ChannelNature RBRGen4ChannelNature_parse(
    const char *value)
{
    for (int32_t nature = 0;
         nature < RBRGEN4_CHANNEL_NATURE_COUNT;
         ++nature)
    {
        if (strcmp(value,
                   RBRGen4ChannelNature_name(nature)) == 0)
        {
            return nature;
        }
    }

    return RBRGEN4_UNKNOWN_CHANNEL_NATURE;
}

/**
 * \brief Copy a label the instrument reports as `na` as an empty string.
 *
 * A derived channel has no node, port, or device.
 */
static void RBRGen4_copyOptionalLabel(char *destination,
                                                size_t size,
                                                const char *value)
{
    if (strcmp(value, "na") == 0)
    {
        destination[0] = '\0';
        return;
    }

    snprintf(destination, size, "%s", value);
}

/**
 * \brief Read the labels of a `channel` response into a pool.
 */
static RBRGen4Error RBRGen4_parseChannelPool(
    RBRGen4 *instrument,
    RBRGen4ChannelPool *channelPool)
{
    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            channelPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no channels reports `none`, not an empty
             * list. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            /* Channels past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= channelPool->size)
                {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(channelPool->pool[i].label,
                         sizeof(channelPool->pool[i].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_getChannel(
    RBRGen4 *instrument,
    RBRGen4Channel *channel)
{
    if (channel->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(channel, label);

    RBR_TRY(RBRGen4_converse(instrument,
                                       "channel %s",
                                       channel->label));

    channel->nature = RBRGEN4_UNKNOWN_CHANNEL_NATURE;

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "type") == 0)
        {
            snprintf(channel->type,
                     sizeof(channel->type),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "settlingtime") == 0)
        {
            channel->settlingTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "measuringtime") == 0)
        {
            channel->measuringTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "readouttime") == 0)
        {
            channel->readOutTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "userunits") == 0)
        {
            snprintf(channel->userUnits,
                     sizeof(channel->userUnits),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "nature") == 0)
        {
            channel->nature =
                RBRGen4ChannelNature_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "derived") == 0)
        {
            channel->derived = (strcmp(parameter.value, "true") == 0);
        }
        else if (strcmp(parameter.key, "node") == 0)
        {
            RBRGen4_copyOptionalLabel(channel->node,
                                                sizeof(channel->node),
                                                parameter.value);
        }
        else if (strcmp(parameter.key, "port") == 0)
        {
            RBRGen4_copyOptionalLabel(channel->port,
                                                sizeof(channel->port),
                                                parameter.value);
        }
        else if (strcmp(parameter.key, "device") == 0)
        {
            RBRGen4_copyOptionalLabel(channel->device,
                                                sizeof(channel->device),
                                                parameter.value);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setChannel(
    RBRGen4 *instrument,
    const RBRGen4Channel *channel)
{
    if (channel->userUnits[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument,
                                      "channel %s userunits=%s",
                                      channel->label,
                                      channel->userUnits);
}

RBRGen4Error RBRGen4_getChannelPool(
    RBRGen4 *instrument,
    RBRGen4ChannelPool *channelPool)
{
    channelPool->count = 0;
    memset(channelPool->pool,
           0,
           channelPool->size * sizeof(RBRGen4Channel));

    RBR_TRY(RBRGen4_converse(instrument, "channel"));

    return RBRGen4_parseChannelPool(instrument, channelPool);
}

RBRGen4Error RBRGen4_getChannelPoolByNature(
    RBRGen4 *instrument,
    RBRGen4ChannelNature nature,
    RBRGen4ChannelPool *channelPool)
{
    if (nature < 0 || nature >= RBRGEN4_CHANNEL_NATURE_COUNT)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    channelPool->count = 0;
    memset(channelPool->pool,
           0,
           channelPool->size * sizeof(RBRGen4Channel));

    RBR_TRY(RBRGen4_converse(
                instrument,
                "channel %s",
                RBRGen4ChannelNature_name(nature)));

    return RBRGen4_parseChannelPool(instrument, channelPool);
}

RBRGen4Error RBRGen4_getSettings(
    RBRGen4 *instrument,
    RBRGen4Settings *settings)
{
    memset(settings, 0, sizeof(RBRGen4Settings));

    RBR_TRY(RBRGen4_converse(instrument, "settings"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "prompt") == 0)
        {
            settings->prompt = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "confirmation") == 0)
        {
            settings->confirmation = (strcmp(parameter.value, "on") == 0);
        }
        else if (strcmp(parameter.key, "pollpoweroffdelay") == 0)
        {
            settings->pollPowerOffDelay = strtol(parameter.value, NULL, 10);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setSettings(
    RBRGen4 *instrument,
    const RBRGen4Settings *settings)
{
    if (settings->pollPowerOffDelay < 0)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    /* The instrument answers with nothing at all once confirmation is off. */
    if (!settings->confirmation)
    {
        return RBRGen4_sendCommand(
            instrument,
            "settings prompt=%s confirmation=off pollpoweroffdelay=%" PRId32,
            settings->prompt ? "on" : "off",
            settings->pollPowerOffDelay);
    }

    return RBRGen4_converse(
        instrument,
        "settings prompt=%s confirmation=on pollpoweroffdelay=%" PRId32,
        settings->prompt ? "on" : "off",
        settings->pollPowerOffDelay);
}

RBRGen4Error RBRGen4_getParameters(
    RBRGen4 *instrument,
    RBRGen4Parameters *parameters)
{
    memset(parameters, 0, sizeof(RBRGen4Parameters));

    RBR_TRY(RBRGen4_converse(instrument, "parameters"));

    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "altitude") == 0)
        {
            parameters->altitude = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "atmosphere") == 0)
        {
            parameters->atmosphere = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "avgsoundspeed") == 0)
        {
            parameters->avgSoundSpeed = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "density") == 0)
        {
            parameters->density = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "pressure") == 0)
        {
            parameters->pressure = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "salinity") == 0)
        {
            parameters->salinity = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "speccondtempco") == 0)
        {
            parameters->specCondTempCo = strtof(parameter.value, NULL);
        }
        else if (strcmp(parameter.key, "temperature") == 0)
        {
            parameters->temperature = strtof(parameter.value, NULL);
        }
    }

    return RBRGEN4_SUCCESS;
}

RBRGen4Error RBRGen4_setParameters(
    RBRGen4 *instrument,
    const RBRGen4Parameters *parameters)
{
    /* The instrument bounds these values; %.9g round-trips a float. */
    return RBRGen4_converse(
        instrument,
        "parameters altitude=%.9g atmosphere=%.9g avgsoundspeed=%.9g "
        "density=%.9g pressure=%.9g salinity=%.9g speccondtempco=%.9g "
        "temperature=%.9g",
        (double) parameters->altitude,
        (double) parameters->atmosphere,
        (double) parameters->avgSoundSpeed,
        (double) parameters->density,
        (double) parameters->pressure,
        (double) parameters->salinity,
        (double) parameters->specCondTempCo,
        (double) parameters->temperature);
}


RBRGen4Error RBRGen4_getGroup(
    RBRGen4 *instrument,
    RBRGen4Group *group,
    RBRGen4LabelList *channelList)
{
    if (group->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(group, label);
    if (channelList != NULL)
    {
        channelList->count = 0;
    }

    RBR_TRY(RBRGen4_converse(instrument, "group %s", group->label));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "channellist") == 0
                 && channelList != NULL)
        {
            err = RBRGen4_copyLabelList(channelList,
                                                  parameter.value);
        }
    }

    return err;
}

RBRGen4Error RBRGen4_setGroup(
    RBRGen4 *instrument,
    const RBRGen4Group *group,
    const RBRGen4LabelList *channelList)
{
    if (group->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    char value[RBRGEN4_COMMAND_BUFFER_MAX];
    RBR_TRY(RBRGen4_formatLabelList(value,
                                              (int32_t) sizeof(value),
                                              channelList));

    return RBRGen4_converse(instrument,
                                      "group %s channellist=%s",
                                      group->label,
                                      value);
}

RBRGen4Error RBRGen4_getGroupPool(
    RBRGen4 *instrument,
    RBRGen4GroupPool *groupPool)
{
    groupPool->count = 0;
    groupPool->maxCount = 0;
    memset(groupPool->pool,
           0,
           groupPool->size * sizeof(RBRGen4Group));

    RBR_TRY(RBRGen4_converse(instrument, "group"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            groupPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "maxcount") == 0)
        {
            groupPool->maxCount = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            /* Groups past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= groupPool->size)
                {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(groupPool->pool[i].label,
                         sizeof(groupPool->pool[i].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_createGroup(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "group create %s", label);
}

RBRGen4Error RBRGen4_deleteGroup(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "group delete %s", label);
}

RBRGen4Error RBRGen4_deleteGroupAll(
    RBRGen4 *instrument)
{
    return RBRGen4_converse(instrument, "group delete all");
}

RBRGen4Error RBRGen4_getConfig(
    RBRGen4 *instrument,
    RBRGen4Config *config,
    RBRGen4LabelList *scheduleList)
{
    if (config->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(config, label);
    if (scheduleList != NULL)
    {
        scheduleList->count = 0;
    }

    RBR_TRY(RBRGen4_converse(instrument,
                                       "config %s",
                                       config->label));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "schedulelist") == 0
                 && scheduleList != NULL)
        {
            err = RBRGen4_copyLabelList(scheduleList,
                                                  parameter.value);
        }
    }

    return err;
}

RBRGen4Error RBRGen4_setConfig(
    RBRGen4 *instrument,
    const RBRGen4Config *config,
    const RBRGen4LabelList *scheduleList)
{
    if (config->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    char value[RBRGEN4_COMMAND_BUFFER_MAX];
    RBR_TRY(RBRGen4_formatLabelList(value,
                                              (int32_t) sizeof(value),
                                              scheduleList));

    return RBRGen4_converse(instrument,
                                      "config %s schedulelist=%s",
                                      config->label,
                                      value);
}

RBRGen4Error RBRGen4_getConfigPool(
    RBRGen4 *instrument,
    RBRGen4ConfigPool *configPool)
{
    configPool->count = 0;
    configPool->maxCount = 0;
    memset(configPool->pool,
           0,
           configPool->size * sizeof(RBRGen4Config));

    RBR_TRY(RBRGen4_converse(instrument, "config"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            configPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "maxcount") == 0)
        {
            configPool->maxCount = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            /* Configurations past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= configPool->size)
                {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(configPool->pool[i].label,
                         sizeof(configPool->pool[i].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_createConfig(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "config create %s", label);
}

RBRGen4Error RBRGen4_deleteConfig(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "config delete %s", label);
}

RBRGen4Error RBRGen4_deleteConfigAll(
    RBRGen4 *instrument)
{
    return RBRGen4_converse(instrument, "config delete all");
}

const char *RBRGen4ScheduleMode_name(
    RBRGen4ScheduleMode mode)
{
    switch (mode)
    {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        return "continuous";
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
        return "average";
    case RBRGEN4_SCHEDULE_MODE_BURST:
        return "burst";
    case RBRGEN4_SCHEDULE_MODE_TIDE:
        return "tide";
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        return "wave";
    case RBRGEN4_SCHEDULE_MODE_DDSAMPLING:
        return "ddsampling";
    case RBRGEN4_SCHEDULE_MODE_REGIMES:
        return "regimes";
    case RBRGEN4_SCHEDULE_MODE_NONE:
    default:
        return "unknown schedule mode";
    }
}

const char *RBRGen4ScheduleStorage_name(
    RBRGen4ScheduleStorage storage)
{
    switch (storage)
    {
    case RBRGEN4_SCHEDULE_STORAGE_OFF:
        return "off";
    case RBRGEN4_SCHEDULE_STORAGE_ON:
        return "on";
    case RBRGEN4_SCHEDULE_STORAGE_COUNT:
        return "schedule storage count";
    case RBRGEN4_UNKNOWN_SCHEDULE_STORAGE:
    default:
        return "unknown schedule storage";
    }
}

/**
 * \brief Find the stream destination a response value names.
 *
 * \param [in] value the response value
 * \return the destination, or #RBRGEN4_UNKNOWN_SCHEDULE_STREAM
 */
static RBRGen4ScheduleStream RBRGen4ScheduleStream_parse(
    const char *value)
{
    for (int i = 0; i < RBRGEN4_SCHEDULE_STREAM_COUNT; i++)
    {
        if (strcmp(RBRGen4ScheduleStream_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_UNKNOWN_SCHEDULE_STREAM;
}

/**
 * \brief Find the schedule mode a response value names.
 *
 * \param [in] value the response value
 * \return the mode, or #RBRGEN4_SCHEDULE_MODE_NONE
 */
static RBRGen4ScheduleMode RBRGen4ScheduleMode_parse(
    const char *value)
{
    for (int i = RBRGEN4_SCHEDULE_MODE_NONE + 1;
         i <= RBRGEN4_SCHEDULE_MODE_MAX;
         i <<= 1)
    {
        if (strcmp(RBRGen4ScheduleMode_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRGEN4_SCHEDULE_MODE_NONE;
}

RBRGen4Error RBRGen4_getSchedule(
    RBRGen4 *instrument,
    RBRGen4Schedule *schedule,
    RBRGen4LabelList *groupList)
{
    if (schedule->label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBR_RESET_EXCEPT(schedule, label);
    if (groupList != NULL)
    {
        groupList->count = 0;
    }

    RBR_TRY(RBRGen4_converse(instrument,
                                       "schedule %s",
                                       schedule->label));

    schedule->stream = RBRGEN4_UNKNOWN_SCHEDULE_STREAM;
    schedule->storage = RBRGEN4_UNKNOWN_SCHEDULE_STORAGE;
    schedule->mode = RBRGEN4_SCHEDULE_MODE_NONE;

    RBRGen4Period period = 0;
    RBRGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "grouplist") == 0
                 && groupList != NULL)
        {
            err = RBRGen4_copyLabelList(groupList, parameter.value);
        }
        else if (strcmp(parameter.key, "stream") == 0)
        {
            schedule->stream = RBRGen4ScheduleStream_parse(
                parameter.value);
        }
        else if (strcmp(parameter.key, "storage") == 0)
        {
            schedule->storage = strcmp(parameter.value, "on") == 0
                                ? RBRGEN4_SCHEDULE_STORAGE_ON
                                : RBRGEN4_SCHEDULE_STORAGE_OFF;
        }
        else if (strcmp(parameter.key, "castdetection") == 0)
        {
            schedule->castDetection = strcmp(parameter.value, "on") == 0;
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            schedule->mode = RBRGen4ScheduleMode_parse(
                parameter.value);
        }
        else if (strcmp(parameter.key, "period") == 0)
        {
            period = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "measurementperiod") == 0)
        {
            measurementPeriod = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "measurementcount") == 0)
        {
            measurementCount = strtol(parameter.value, NULL, 10);
        }
    }

    switch (schedule->mode)
    {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        schedule->parameters.continuous.period = period;
        break;
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRGEN4_SCHEDULE_MODE_BURST:
    case RBRGEN4_SCHEDULE_MODE_TIDE:
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        schedule->parameters.bursting.period = period;
        schedule->parameters.bursting.measurementPeriod = measurementPeriod;
        schedule->parameters.bursting.measurementCount = measurementCount;
        break;
    default:
        break;
    }

    return err;
}

RBRGen4Error RBRGen4_setSchedule(
    RBRGen4 *instrument,
    const RBRGen4Schedule *schedule,
    const RBRGen4LabelList *groupList)
{
    /* A schedule runs in exactly one mode. */
    if (schedule->label[0] == '\0'
        || schedule->stream >= RBRGEN4_SCHEDULE_STREAM_COUNT
        || schedule->storage > RBRGEN4_UNKNOWN_SCHEDULE_STORAGE
        || schedule->storage == RBRGEN4_SCHEDULE_STORAGE_COUNT
        || schedule->mode == RBRGEN4_SCHEDULE_MODE_NONE
        || schedule->mode > RBRGEN4_SCHEDULE_MODE_MAX
        || (schedule->mode & (schedule->mode - 1)) != 0)
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRGen4Period period;
    RBRGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

    switch (schedule->mode)
    {
    case RBRGEN4_SCHEDULE_MODE_CONTINUOUS:
        period = schedule->parameters.continuous.period;
        break;
    case RBRGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRGEN4_SCHEDULE_MODE_BURST:
    case RBRGEN4_SCHEDULE_MODE_TIDE:
    case RBRGEN4_SCHEDULE_MODE_WAVE:
        period = schedule->parameters.bursting.period;
        measurementPeriod = schedule->parameters.bursting.measurementPeriod;
        measurementCount = schedule->parameters.bursting.measurementCount;
        break;
    default:
        /* No parameters are modelled for `ddsampling` or `regimes`, so
         * writing either would silently drop them. */
        return RBRGEN4_UNSUPPORTED;
    }

    /* A NULL list leaves the instrument's group list unchanged. */
    char groups[RBRGEN4_COMMAND_BUFFER_MAX] = "";
    if (groupList != NULL)
    {
        const char prefix[] = "grouplist=";
        memcpy(groups, prefix, sizeof(prefix));
        int32_t length = (int32_t) sizeof(prefix) - 1;
        /* Leave room for the separating space. */
        RBR_TRY(RBRGen4_formatLabelList(
                    groups + length,
                    (int32_t) sizeof(groups) - length - 1,
                    groupList));
        strcat(groups, " ");
    }

    /* Sending a `storage` the getter never read would be an error. */
    const char *storage = "";
    if (schedule->storage != RBRGEN4_UNKNOWN_SCHEDULE_STORAGE)
    {
        storage = schedule->storage == RBRGEN4_SCHEDULE_STORAGE_ON
                  ? "storage=on "
                  : "storage=off ";
    }

/* The parameters every mode sends, shared by the two forms below so the
 * spelling cannot drift between them. */
#define SCHEDULE_COMMON \
    "schedule %s %sstream=%s %scastdetection=%s mode=%s"

    if (schedule->mode == RBRGEN4_SCHEDULE_MODE_CONTINUOUS)
    {
        return RBRGen4_converse(
            instrument,
            SCHEDULE_COMMON " period=%" PRId32,
            schedule->label,
            groups,
            RBRGen4ScheduleStream_name(schedule->stream),
            storage,
            schedule->castDetection ? "on" : "off",
            RBRGen4ScheduleMode_name(schedule->mode),
            period);
    }

    return RBRGen4_converse(
        instrument,
        SCHEDULE_COMMON
        " period=%" PRId32
        " measurementcount=%" PRId32
        " measurementperiod=%" PRId32,
        schedule->label,
        groups,
        RBRGen4ScheduleStream_name(schedule->stream),
        storage,
        schedule->castDetection ? "on" : "off",
        RBRGen4ScheduleMode_name(schedule->mode),
        period,
        measurementCount,
        measurementPeriod);

#undef SCHEDULE_COMMON
}

RBRGen4Error RBRGen4_getSchedulePool(
    RBRGen4 *instrument,
    RBRGen4SchedulePool *schedulePool)
{
    schedulePool->count = 0;
    schedulePool->maxCount = 0;
    memset(schedulePool->pool,
           0,
           schedulePool->size * sizeof(RBRGen4Schedule));
    schedulePool->availableModes = RBRGEN4_SCHEDULE_MODE_NONE;
    schedulePool->availableFastPeriodCount = 0;
    memset(schedulePool->availableFastPeriods,
           0,
           sizeof(schedulePool->availableFastPeriods));
    schedulePool->maxRegimes = 0;

    RBR_TRY(RBRGen4_converse(instrument, "schedule"));

    RBRGen4Error err = RBRGEN4_SUCCESS;
    char *command = NULL;
    RBRGen4ResponseParameter parameter;
    while (true)
    {
        RBRGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);

        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "count") == 0)
        {
            schedulePool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "maxcount") == 0)
        {
            schedulePool->maxCount = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "maxregimes") == 0)
        {
            schedulePool->maxRegimes = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An empty pool reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            /* Schedules past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= schedulePool->size)
                {
                    err = RBRGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRGen4_splitListValue(value);

                snprintf(schedulePool->pool[i].label,
                         sizeof(schedulePool->pool[i].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "availablemodes") == 0)
        {
            char *value = parameter.value;
            while (value != NULL)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                /* An unrecognized mode parses to `NONE` and drops out. */
                schedulePool->availableModes |=
                    RBRGen4ScheduleMode_parse(value);

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "availablefastperiods") == 0)
        {
            /* No fast periods reports `none`. */
            if (strcmp(parameter.value, RBRGEN4_EMPTY_LIST) == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && schedulePool->availableFastPeriodCount
                   < RBRGEN4_AVAILABLE_FAST_PERIODS_MAX)
            {
                char *nextValue = RBRGen4_splitListValue(value);

                schedulePool->availableFastPeriods[
                    schedulePool->availableFastPeriodCount] =
                    strtol(value, NULL, 10);
                schedulePool->availableFastPeriodCount++;

                value = nextValue;
            }
        }
    }

    return err;
}

RBRGen4Error RBRGen4_createSchedule(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "schedule create %s", label);
}

RBRGen4Error RBRGen4_deleteSchedule(
    RBRGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRGen4_converse(instrument, "schedule delete %s", label);
}

RBRGen4Error RBRGen4_deleteScheduleAll(
    RBRGen4 *instrument)
{
    return RBRGen4_converse(instrument, "schedule delete all");
}
