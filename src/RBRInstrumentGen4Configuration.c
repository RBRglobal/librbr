/**
 * \file RBRInstrumentGen4Configuration.c
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

#include "RBRInstrumentGen4.h"
#include "RBRInstrumentGen4Internal.h"
#include "RBRInstrumentGen4Configuration.h"

RBRInstrumentGen4Error RBRInstrumentGen4_getNode(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Node *node)
{
    /* The label selects the node to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(node->label)];
    snprintf(label, sizeof(label), "%s", node->label);

    memset(node, 0, sizeof(RBRInstrumentGen4Node));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "node %s", label));

    snprintf(node->label, sizeof(node->label), "%s", label);

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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && node->portCount < RBRINSTRUMENTGEN4_PORT_COUNT_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getNodePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4NodePool *nodePool)
{
    memset(nodePool, 0, sizeof(RBRInstrumentGen4NodePool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "node"));

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
        else if (strcmp(parameter.key, "count") == 0)
        {
            nodePool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no nodes reports `none` to indicate an empty list */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t node = 0;
                 value != NULL && node < RBRINSTRUMENTGEN4_NODE_COUNT_MAX;
                 node++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(nodePool->pool[node].label,
                         sizeof(nodePool->pool[node].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4PortClass_name(
    RBRInstrumentGen4PortClass portClass)
{
    switch (portClass)
    {
    case RBRINSTRUMENTGEN4_PORT_CLASS_VIRTUAL:
        return "virtual";
    case RBRINSTRUMENTGEN4_PORT_CLASS_SERIAL:
        return "serial";
    case RBRINSTRUMENTGEN4_PORT_CLASS_COUNT:
        return "port class count";
    case RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS:
    default:
        return "unknown port class";
    }
}

const char *RBRInstrumentGen4PortProtocol_name(
    RBRInstrumentGen4PortProtocol protocol)
{
    switch (protocol)
    {
    case RBRINSTRUMENTGEN4_PORT_PROTOCOL_PRESSURE:
        return "pressure";
    case RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRSERIAL:
        return "rbrserial";
    case RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMODEM:
        return "rbrmodem";
    case RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMULTIDROP:
        return "rbrmultidrop";
    case RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE:
    default:
        return "none";
    }
}

/**
 * \brief Find the port protocol a response value names.
 *
 * \param [in] value the response value
 * \return the protocol, or #RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE
 */
static RBRInstrumentGen4PortProtocol RBRInstrumentGen4PortProtocol_parse(
    const char *value)
{
    for (int i = RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE + 1;
         i <= RBRINSTRUMENTGEN4_PORT_PROTOCOL_MAX;
         i <<= 1)
    {
        if (strcmp(RBRInstrumentGen4PortProtocol_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPort(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Port *port)
{
    /* The label selects the port to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(port->label)];
    snprintf(label, sizeof(label), "%s", port->label);

    memset(port, 0, sizeof(RBRInstrumentGen4Port));
    port->portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS;

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "port %s", label));

    snprintf(port->label, sizeof(port->label), "%s", label);

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
        else if (strcmp(parameter.key, "node") == 0)
        {
            snprintf(port->node,
                     sizeof(port->node),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "class") == 0)
        {
            for (int32_t i = 0; i < RBRINSTRUMENTGEN4_PORT_CLASS_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4PortClass_name(i),
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
                = RBRInstrumentGen4PortProtocol_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "availableprotocols") == 0)
        {
            char *value = parameter.value;
            while (value != NULL)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);
                port->availableProtocols
                    |= RBRInstrumentGen4PortProtocol_parse(value);

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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && port->deviceCount < RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getPortPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PortPool *portPool)
{
    memset(portPool, 0, sizeof(RBRInstrumentGen4PortPool));

    /* Zero is a real port class, so say the class is unknown until
     * RBRInstrumentGen4_getPort() reads it. */
    for (int32_t port = 0; port < RBRINSTRUMENTGEN4_PORT_COUNT_MAX; port++)
    {
        portPool->pool[port].portClass = RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS;
    }

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "port"));

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
        else if (strcmp(parameter.key, "count") == 0)
        {
            portPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no ports reports `none`, not an empty list. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t port = 0;
                 value != NULL && port < RBRINSTRUMENTGEN4_PORT_COUNT_MAX;
                 port++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(portPool->pool[port].label,
                         sizeof(portPool->pool[port].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

const char *RBRInstrumentGen4DeviceClass_name(
    RBRInstrumentGen4DeviceClass deviceClass)
{
    switch (deviceClass)
    {
    case RBRINSTRUMENTGEN4_DEVICE_CLASS_SENSOR:
        return "sensor";
    case RBRINSTRUMENTGEN4_DEVICE_CLASS_ACTUATOR:
        return "actuator";
    case RBRINSTRUMENTGEN4_DEVICE_CLASS_EVENTGEN:
        return "eventgen";
    case RBRINSTRUMENTGEN4_DEVICE_CLASS_MODEM:
        return "modem";
    case RBRINSTRUMENTGEN4_DEVICE_CLASS_COUNT:
        return "device class count";
    case RBRINSTRUMENTGEN4_UNKNOWN_DEVICE_CLASS:
    default:
        return "unknown device class";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDevice(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Device *device)
{
    /* The label selects the device to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(device->label)];
    snprintf(label, sizeof(label), "%s", device->label);

    memset(device, 0, sizeof(RBRInstrumentGen4Device));
    device->deviceClass = RBRINSTRUMENTGEN4_UNKNOWN_DEVICE_CLASS;

    /* `lock` is deliberately absent from the response to a bare
     * `device <label>`, so name every parameter rather than take the
     * defaults. */
    RBR_TRY(RBRInstrumentGen4_converse(
        instrument,
        "device %s port class sn pn fwversion fwtype name channellist lock"
        " poweruptime cooldowntime powerdowntime inrushoffsettime",
        label));

    snprintf(device->label, sizeof(device->label), "%s", label);

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
        else if (strcmp(parameter.key, "port") == 0)
        {
            snprintf(device->port,
                     sizeof(device->port),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "class") == 0)
        {
            for (int32_t i = 0; i < RBRINSTRUMENTGEN4_DEVICE_CLASS_COUNT; i++)
            {
                if (strcmp(RBRInstrumentGen4DeviceClass_name(i),
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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && device->channelCount < RBRINSTRUMENTGEN4_CHANNEL_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getDevicePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DevicePool *devicePool)
{
    memset(devicePool, 0, sizeof(RBRInstrumentGen4DevicePool));

    /* Zero is a real device class, so say the class is unknown until
     * RBRInstrumentGen4_getDevice() reads it. */
    for (int32_t device = 0;
         device < RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX;
         device++)
    {
        devicePool->pool[device].deviceClass
            = RBRINSTRUMENTGEN4_UNKNOWN_DEVICE_CLASS;
    }

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "device"));

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
        else if (strcmp(parameter.key, "count") == 0)
        {
            devicePool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no devices reports `none`, not an empty
             * list. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t device = 0;
                 value != NULL && device < RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX;
                 device++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(devicePool->pool[device].label,
                         sizeof(devicePool->pool[device].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_discoverDevices(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DevicePool *devicePool)
{
    memset(devicePool, 0, sizeof(RBRInstrumentGen4DevicePool));

    /* Zero is a real device class, so say the class is unknown until
     * RBRInstrumentGen4_getDevice() reads it. */
    for (int32_t device = 0;
         device < RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX;
         device++)
    {
        devicePool->pool[device].deviceClass
            = RBRINSTRUMENTGEN4_UNKNOWN_DEVICE_CLASS;
    }

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "device discover"));

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
        else if (strcmp(parameter.key, "found") == 0)
        {
            /* Discovery finding nothing reports `none`, not an empty list.
             * There is no count to read: the list is the whole answer. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && devicePool->count < RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(devicePool->pool[devicePool->count].label,
                         sizeof(devicePool->pool[devicePool->count].label),
                         "%s",
                         value);
                devicePool->count++;

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Calibration *calibration)
{
    /* The label selects the calibration to read, so it has to outlive the
     * reset of the rest of the structure. */
    char label[sizeof(calibration->label)];
    snprintf(label, sizeof(label), "%s", calibration->label);

    memset(calibration, 0, sizeof(RBRInstrumentGen4Calibration));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "calibration %s", label));

    snprintf(calibration->label, sizeof(calibration->label), "%s", label);

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

        /* A coefficient key is a group letter and an index: `a0`, `m11`. */
        char *end = NULL;
        char group = parameter.key[0];
        int32_t index = strtol(parameter.key + 1, &end, 10);
        bool coefficient =
            (parameter.key[1] != '\0'
             && *end == '\0'
             && index >= 0
             && index < RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX);

        if (strcmp(parameter.key, "equation") == 0)
        {
            snprintf((char *) calibration->equation,
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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Append one `<group><index>=<value>` coefficient to a command.
 *
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the coefficient does not
 *                                             fit
 */
static RBRInstrumentGen4Error RBRInstrumentGen4Calibration_appendCoefficient(
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
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    int32_t written = snprintf(command + *length,
                               size - *length,
                               " %c%" PRId32 "=%.9g",
                               group,
                               index,
                               (double) value);

    if (written < 0 || *length + written >= size)
    {
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    *length += written;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

/**
 * \brief Append one `m<index>=<label>` reference to a command.
 *
 * An empty label is sent as `none`, which is how the instrument reports a
 * reference the equation does not use.
 *
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the reference does not fit
 */
static RBRInstrumentGen4Error RBRInstrumentGen4Calibration_appendReference(
    char *command,
    int32_t size,
    int32_t *length,
    int32_t index,
    const char *label)
{
    if (*length < 0 || *length >= size)
    {
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    int32_t written = snprintf(command + *length,
                               size - *length,
                               " m%" PRId32 "=%s",
                               index,
                               label[0] == '\0' ? "none" : label);

    if (written < 0 || *length + written >= size)
    {
        return RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL;
    }

    *length += written;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Calibration *calibration)
{
    if (calibration->aCount < 0
        || calibration->aCount > RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX
        || calibration->bCount < 0
        || calibration->bCount > RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX
        || calibration->mCount < 0
        || calibration->mCount > RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char coefficients[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX] = "";
    int32_t length = 0;

    for (int32_t a = 0; a < calibration->aCount; ++a)
    {
        RBR_TRY(RBRInstrumentGen4Calibration_appendCoefficient(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    'a',
                    a,
                    calibration->a[a]));
    }
    for (int32_t b = 0; b < calibration->bCount; ++b)
    {
        RBR_TRY(RBRInstrumentGen4Calibration_appendCoefficient(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    'b',
                    b,
                    calibration->b[b]));
    }
    for (int32_t m = 0; m < calibration->mCount; ++m)
    {
        RBR_TRY(RBRInstrumentGen4Calibration_appendReference(
                    coefficients,
                    (int32_t) sizeof(coefficients),
                    &length,
                    m,
                    calibration->m[m]));
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "calibration %s datetime=%014" PRId64 " offset=%.9g slope=%.9g%s",
        calibration->label,
        calibration->dateTime,
        (double) calibration->userOffset,
        (double) calibration->userSlope,
        coefficients);
}

const char *RBRInstrumentGen4ScheduleStream_name(
    RBRInstrumentGen4ScheduleStream stream)
{
    switch (stream)
    {
    case RBRINSTRUMENTGEN4_SCHEDULE_STREAM_OFF:
        return "off";
    case RBRINSTRUMENTGEN4_SCHEDULE_STREAM_USB:
        return "usb";
    case RBRINSTRUMENTGEN4_SCHEDULE_STREAM_SERIAL:
        return "serial";
    case RBRINSTRUMENTGEN4_SCHEDULE_STREAM_COUNT:
        return "stream destination count";
    case RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM:
    default:
        return "unknown stream destination";
    }
}

const char *RBRInstrumentGen4ChannelNature_name(
    RBRInstrumentGen4ChannelNature nature)
{
    switch (nature)
    {
    case RBRINSTRUMENTGEN4_CHANNEL_NATURE_SCIENTIFIC:
        return "scientific";
    case RBRINSTRUMENTGEN4_CHANNEL_NATURE_SYSTEM:
        return "system";
    case RBRINSTRUMENTGEN4_CHANNEL_NATURE_COUNT:
        return "channel nature count";
    case RBRINSTRUMENTGEN4_UNKNOWN_CHANNEL_NATURE:
    default:
        return "unknown channel nature";
    }
}

static RBRInstrumentGen4ChannelNature RBRInstrumentGen4ChannelNature_parse(
    const char *value)
{
    for (int32_t nature = 0;
         nature < RBRINSTRUMENTGEN4_CHANNEL_NATURE_COUNT;
         ++nature)
    {
        if (strcmp(value,
                   RBRInstrumentGen4ChannelNature_name(nature)) == 0)
        {
            return nature;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_CHANNEL_NATURE;
}

/**
 * \brief Copy a label the instrument reports as `na` as an empty string.
 *
 * A derived channel has no node, port, or device.
 */
static void RBRInstrumentGen4_copyOptionalLabel(char *destination,
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
static RBRInstrumentGen4Error RBRInstrumentGen4_parseChannelPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool)
{
    RBRInstrumentGen4Error err = RBRINSTRUMENTGEN4_SUCCESS;
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
        else if (strcmp(parameter.key, "count") == 0)
        {
            channelPool->count = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "list") == 0)
        {
            /* An instrument with no channels reports `none`, not an empty
             * list. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            /* Channels past the pool's capacity are discarded. */
            char *value = parameter.value;
            for (int32_t i = 0; value != NULL; i++)
            {
                if (i >= channelPool->size)
                {
                    err = RBRINSTRUMENTGEN4_TRUNCATED;
                    break;
                }

                char *nextValue = RBRInstrumentGen4_splitListValue(value);

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

RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Channel *channel)
{
    /* The label selects the channel to read, so it has to outlive the reset of
     * the rest of the structure. */
    char label[sizeof(channel->label)];
    snprintf(label, sizeof(label), "%s", channel->label);

    memset(channel, 0, sizeof(RBRInstrumentGen4Channel));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "channel %s", label));

    snprintf(channel->label, sizeof(channel->label), "%s", label);
    *(RBRInstrumentGen4ChannelNature *) &channel->nature =
        RBRINSTRUMENTGEN4_UNKNOWN_CHANNEL_NATURE;

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
        else if (strcmp(parameter.key, "type") == 0)
        {
            snprintf((char *) channel->type,
                     sizeof(channel->type),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "settlingtime") == 0)
        {
            *(RBRInstrumentGen4Period *) &channel->settlingTime =
                strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "measuringtime") == 0)
        {
            *(RBRInstrumentGen4Period *) &channel->measuringTime =
                strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "readouttime") == 0)
        {
            *(RBRInstrumentGen4Period *) &channel->readOutTime =
                strtol(parameter.value, NULL, 10);
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
            *(RBRInstrumentGen4ChannelNature *) &channel->nature =
                RBRInstrumentGen4ChannelNature_parse(parameter.value);
        }
        else if (strcmp(parameter.key, "derived") == 0)
        {
            *(bool *) &channel->derived = (strcmp(parameter.value,
                                                  "true") == 0);
        }
        else if (strcmp(parameter.key, "node") == 0)
        {
            RBRInstrumentGen4_copyOptionalLabel((char *) channel->node,
                                                sizeof(channel->node),
                                                parameter.value);
        }
        else if (strcmp(parameter.key, "port") == 0)
        {
            RBRInstrumentGen4_copyOptionalLabel((char *) channel->port,
                                                sizeof(channel->port),
                                                parameter.value);
        }
        else if (strcmp(parameter.key, "device") == 0)
        {
            RBRInstrumentGen4_copyOptionalLabel((char *) channel->device,
                                                sizeof(channel->device),
                                                parameter.value);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setChannel(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Channel *channel)
{
    if (channel->userUnits[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument,
                                      "channel %s userunits=%s",
                                      channel->label,
                                      channel->userUnits);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool)
{
    channelPool->count = 0;
    memset(channelPool->pool,
           0,
           channelPool->size * sizeof(RBRInstrumentGen4Channel));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "channel"));

    return RBRInstrumentGen4_parseChannelPool(instrument, channelPool);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPoolByNature(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelNature nature,
    RBRInstrumentGen4ChannelPool *channelPool)
{
    if (nature < 0 || nature >= RBRINSTRUMENTGEN4_CHANNEL_NATURE_COUNT)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    channelPool->count = 0;
    memset(channelPool->pool,
           0,
           channelPool->size * sizeof(RBRInstrumentGen4Channel));

    RBR_TRY(RBRInstrumentGen4_converse(
                instrument,
                "channel %s",
                RBRInstrumentGen4ChannelNature_name(nature)));

    return RBRInstrumentGen4_parseChannelPool(instrument, channelPool);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings)
{
    memset(settings, 0, sizeof(RBRInstrumentGen4Settings));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "settings"));

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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings)
{
    if (settings->pollPowerOffDelay < 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    /* The instrument answers with nothing at all once confirmation is off. */
    if (!settings->confirmation)
    {
        return RBRInstrumentGen4_sendCommand(
            instrument,
            "settings prompt=%s confirmation=off pollpoweroffdelay=%" PRId32,
            settings->prompt ? "on" : "off",
            settings->pollPowerOffDelay);
    }

    return RBRInstrumentGen4_converse(
        instrument,
        "settings prompt=%s confirmation=on pollpoweroffdelay=%" PRId32,
        settings->prompt ? "on" : "off",
        settings->pollPowerOffDelay);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getParameters(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Parameters *parameters)
{
    memset(parameters, 0, sizeof(RBRInstrumentGen4Parameters));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "parameters"));

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

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setParameters(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Parameters *parameters)
{
    /* The instrument bounds these values; %.9g round-trips a float. */
    return RBRInstrumentGen4_converse(
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


RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Group *group)
{
    /* The label selects the group, so it outlives the reset. */
    char label[sizeof(group->label)];
    snprintf(label, sizeof(label), "%s", group->label);

    memset(group, 0, sizeof(RBRInstrumentGen4Group));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "group %s", label));

    snprintf(group->label, sizeof(group->label), "%s", label);

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
        else if (strcmp(parameter.key, "channellist") == 0)
        {
            RBRInstrumentGen4_parseLabelList(group->channelList,
                                             RBRINSTRUMENTGEN4_CHANNEL_MAX,
                                             &group->channelCount,
                                             parameter.value);
        }
        else if (strcmp(parameter.key, "schedulelist") == 0)
        {
            RBRInstrumentGen4_parseLabelList(
                (RBRInstrumentGen4Label *) group->scheduleList,
                RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX,
                (int32_t *) &group->scheduleCount,
                parameter.value);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Group *group)
{
    if (group->label[0] == '\0'
        || group->channelCount < 0
        || group->channelCount > RBRINSTRUMENTGEN4_CHANNEL_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char channelList[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX];
    RBR_TRY(RBRInstrumentGen4_formatLabelList(channelList,
                                              (int32_t) sizeof(channelList),
                                              group->channelList,
                                              group->channelCount));

    return RBRInstrumentGen4_converse(instrument,
                                      "group %s channellist=%s",
                                      group->label,
                                      channelList);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getGroupPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4GroupPool *groupPool)
{
    memset(groupPool, 0, sizeof(RBRInstrumentGen4GroupPool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "group"));

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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t group = 0;
                 value != NULL && group < RBRINSTRUMENTGEN4_GROUP_COUNT_MAX;
                 group++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(groupPool->pool[group].label,
                         sizeof(groupPool->pool[group].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "group create %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "group delete %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "group delete all");
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Config *config)
{
    /* The label selects the configuration, so it outlives the reset. */
    char label[sizeof(config->label)];
    snprintf(label, sizeof(label), "%s", config->label);

    memset(config, 0, sizeof(RBRInstrumentGen4Config));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "config %s", label));

    snprintf(config->label, sizeof(config->label), "%s", label);

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
        else if (strcmp(parameter.key, "schedulelist") == 0)
        {
            RBRInstrumentGen4_parseLabelList(
                config->scheduleList,
                RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX,
                &config->scheduleCount,
                parameter.value);
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config)
{
    if (config->label[0] == '\0'
        || config->scheduleCount < 0
        || config->scheduleCount > RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    char scheduleList[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX];
    RBR_TRY(RBRInstrumentGen4_formatLabelList(scheduleList,
                                              (int32_t) sizeof(scheduleList),
                                              config->scheduleList,
                                              config->scheduleCount));

    return RBRInstrumentGen4_converse(instrument,
                                      "config %s schedulelist=%s",
                                      config->label,
                                      scheduleList);
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfigPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ConfigPool *configPool)
{
    memset(configPool, 0, sizeof(RBRInstrumentGen4ConfigPool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "config"));

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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t config = 0;
                 value != NULL && config < RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX;
                 config++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(configPool->pool[config].label,
                         sizeof(configPool->pool[config].label),
                         "%s",
                         value);

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "config create %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "config delete %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "config delete all");
}

const char *RBRInstrumentGen4ScheduleMode_name(
    RBRInstrumentGen4ScheduleMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS:
        return "continuous";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_AVERAGE:
        return "average";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_BURST:
        return "burst";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_TIDE:
        return "tide";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_WAVE:
        return "wave";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_DDSAMPLING:
        return "ddsampling";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_REGIMES:
        return "regimes";
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE:
    default:
        return "unknown schedule mode";
    }
}

const char *RBRInstrumentGen4ScheduleStorage_name(
    RBRInstrumentGen4ScheduleStorage storage)
{
    switch (storage)
    {
    case RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_OFF:
        return "off";
    case RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_ON:
        return "on";
    case RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_COUNT:
        return "schedule storage count";
    case RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE:
    default:
        return "unknown schedule storage";
    }
}

/**
 * \brief Find the stream destination a response value names.
 *
 * \param [in] value the response value
 * \return the destination, or #RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM
 */
static RBRInstrumentGen4ScheduleStream RBRInstrumentGen4ScheduleStream_parse(
    const char *value)
{
    for (int i = 0; i < RBRINSTRUMENTGEN4_SCHEDULE_STREAM_COUNT; i++)
    {
        if (strcmp(RBRInstrumentGen4ScheduleStream_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM;
}

/**
 * \brief Find the schedule mode a response value names.
 *
 * \param [in] value the response value
 * \return the mode, or #RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE
 */
static RBRInstrumentGen4ScheduleMode RBRInstrumentGen4ScheduleMode_parse(
    const char *value)
{
    for (int i = RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE + 1;
         i <= RBRINSTRUMENTGEN4_SCHEDULE_MODE_MAX;
         i <<= 1)
    {
        if (strcmp(RBRInstrumentGen4ScheduleMode_name(i), value) == 0)
        {
            return i;
        }
    }

    return RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule)
{
    /* The label selects the schedule, so it outlives the reset. */
    char label[sizeof(schedule->label)];
    snprintf(label, sizeof(label), "%s", schedule->label);

    memset(schedule, 0, sizeof(RBRInstrumentGen4Schedule));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "schedule %s", label));

    snprintf(schedule->label, sizeof(schedule->label), "%s", label);
    schedule->stream = RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM;
    schedule->storage = RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE;
    schedule->mode = RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE;

    RBRInstrumentGen4Period period = 0;
    RBRInstrumentGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

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
        else if (strcmp(parameter.key, "grouplist") == 0)
        {
            RBRInstrumentGen4_parseLabelList(schedule->groupList,
                                             RBRINSTRUMENTGEN4_GROUP_COUNT_MAX,
                                             &schedule->groupCount,
                                             parameter.value);
        }
        else if (strcmp(parameter.key, "configlist") == 0)
        {
            RBRInstrumentGen4_parseLabelList(
                (RBRInstrumentGen4Label *) schedule->configList,
                RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX,
                (int32_t *) &schedule->configCount,
                parameter.value);
        }
        else if (strcmp(parameter.key, "stream") == 0)
        {
            schedule->stream = RBRInstrumentGen4ScheduleStream_parse(
                parameter.value);
        }
        else if (strcmp(parameter.key, "storage") == 0)
        {
            schedule->storage = strcmp(parameter.value, "on") == 0
                                ? RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_ON
                                : RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_OFF;
        }
        else if (strcmp(parameter.key, "castdetection") == 0)
        {
            schedule->castDetection = strcmp(parameter.value, "on") == 0;
        }
        else if (strcmp(parameter.key, "mode") == 0)
        {
            schedule->mode = RBRInstrumentGen4ScheduleMode_parse(
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
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS:
        schedule->parameters.continuous.period = period;
        break;
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_BURST:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_TIDE:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_WAVE:
        schedule->parameters.bursting.period = period;
        schedule->parameters.bursting.measurementPeriod = measurementPeriod;
        schedule->parameters.bursting.measurementCount = measurementCount;
        break;
    default:
        break;
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSchedule(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Schedule *schedule)
{
    /* A schedule runs in exactly one mode. */
    if (schedule->label[0] == '\0'
        || schedule->groupCount < 0
        || schedule->groupCount > RBRINSTRUMENTGEN4_GROUP_COUNT_MAX
        || schedule->stream >= RBRINSTRUMENTGEN4_SCHEDULE_STREAM_COUNT
        || schedule->storage > RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE
        || schedule->storage == RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_COUNT
        || schedule->mode == RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE
        || schedule->mode > RBRINSTRUMENTGEN4_SCHEDULE_MODE_MAX
        || (schedule->mode & (schedule->mode - 1)) != 0)
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    RBRInstrumentGen4Period period;
    RBRInstrumentGen4Period measurementPeriod = 0;
    int32_t measurementCount = 0;

    switch (schedule->mode)
    {
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS:
        period = schedule->parameters.continuous.period;
        break;
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_AVERAGE:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_BURST:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_TIDE:
    case RBRINSTRUMENTGEN4_SCHEDULE_MODE_WAVE:
        period = schedule->parameters.bursting.period;
        measurementPeriod = schedule->parameters.bursting.measurementPeriod;
        measurementCount = schedule->parameters.bursting.measurementCount;
        break;
    default:
        /* No parameters are modelled for `ddsampling` or `regimes`, so
         * writing either would silently drop them. */
        return RBRINSTRUMENTGEN4_UNSUPPORTED;
    }

    char groupList[RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX];
    RBR_TRY(RBRInstrumentGen4_formatLabelList(groupList,
                                              (int32_t) sizeof(groupList),
                                              schedule->groupList,
                                              schedule->groupCount));

    /* Sending a `storage` the getter never read would be an error. */
    const char *storage = "";
    if (schedule->storage != RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE)
    {
        storage = schedule->storage == RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_ON
                  ? "storage=on "
                  : "storage=off ";
    }

/* The parameters every mode sends, shared by the two forms below so the
 * spelling cannot drift between them. */
#define SCHEDULE_COMMON \
    "schedule %s grouplist=%s stream=%s %scastdetection=%s mode=%s"

    if (schedule->mode == RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS)
    {
        return RBRInstrumentGen4_converse(
            instrument,
            SCHEDULE_COMMON " period=%" PRId32,
            schedule->label,
            groupList,
            RBRInstrumentGen4ScheduleStream_name(schedule->stream),
            storage,
            schedule->castDetection ? "on" : "off",
            RBRInstrumentGen4ScheduleMode_name(schedule->mode),
            period);
    }

    return RBRInstrumentGen4_converse(
        instrument,
        SCHEDULE_COMMON
        " period=%" PRId32
        " measurementcount=%" PRId32
        " measurementperiod=%" PRId32,
        schedule->label,
        groupList,
        RBRInstrumentGen4ScheduleStream_name(schedule->stream),
        storage,
        schedule->castDetection ? "on" : "off",
        RBRInstrumentGen4ScheduleMode_name(schedule->mode),
        period,
        measurementCount,
        measurementPeriod);

#undef SCHEDULE_COMMON
}

RBRInstrumentGen4Error RBRInstrumentGen4_getSchedulePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4SchedulePool *schedulePool)
{
    memset(schedulePool, 0, sizeof(RBRInstrumentGen4SchedulePool));

    RBR_TRY(RBRInstrumentGen4_converse(instrument, "schedule"));

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
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            for (int32_t schedule = 0;
                 value != NULL
                 && schedule < RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX;
                 schedule++)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                snprintf(schedulePool->pool[schedule].label,
                         sizeof(schedulePool->pool[schedule].label),
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
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                /* An unrecognized mode parses to `NONE` and drops out. */
                schedulePool->availableModes |=
                    RBRInstrumentGen4ScheduleMode_parse(value);

                value = nextValue;
            }
        }
        else if (strcmp(parameter.key, "availablefastperiods") == 0)
        {
            /* No fast periods reports `none`. */
            if (strcmp(parameter.value, "none") == 0)
            {
                continue;
            }

            char *value = parameter.value;
            while (value != NULL
                   && schedulePool->availableFastPeriodCount
                   < RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX)
            {
                char *nextValue = RBRInstrumentGen4_splitListValue(value);

                schedulePool->availableFastPeriods[
                    schedulePool->availableFastPeriodCount] =
                    strtol(value, NULL, 10);
                schedulePool->availableFastPeriodCount++;

                value = nextValue;
            }
        }
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createSchedule(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "schedule create %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteSchedule(
    RBRInstrumentGen4 *instrument,
    const char *label)
{
    if (label[0] == '\0')
    {
        return RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE;
    }

    return RBRInstrumentGen4_converse(instrument, "schedule delete %s", label);
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteScheduleAll(
    RBRInstrumentGen4 *instrument)
{
    return RBRInstrumentGen4_converse(instrument, "schedule delete all");
}
