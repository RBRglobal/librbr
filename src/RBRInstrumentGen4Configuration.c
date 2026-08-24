/**
 * \file RBRInstrumentGen4Configuration.c
 *
 * \brief Library implementation.
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

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

/*
RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    RBRInstrumentGen4Calibration *calibration)
{
    memset(calibration, 0, sizeof(RBRInstrumentGen4Calibration));

    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "calibration %s",
                                       channelLabel));
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    do
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);
        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "fwversion") == 0)
        {
            snprintf((char *)(id->fwversion),
                     sizeof(id->fwversion),
                     "%s",
                     parameter.value);
        }
        else if (strcmp(parameter.key, "sn") == 0)
        {
            id->sn = strtol(parameter.value, NULL, 10);
        }
    } while (true);

    return RBRINSTRUMENTGEN4_SUCCESS;
}
*/

RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Calibration *calibration)
{
    memset(calibration, 0, sizeof(RBRInstrumentGen4Calibration));

    calibration->userOffset = NAN;
    calibration->userSlope = NAN;

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX;
         ++c)
    {
        calibration->c[c] = NAN;
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX;
         ++x)
    {
        calibration->x[x] = NAN;
    }

    RBRInstrumentGen4Channel *parent = (RBRInstrumentGen4Channel *)(calibration->parent);
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       "calibration %s",
                                       parent->label));
    char *command = NULL;
    RBRInstrumentGen4ResponseParameter parameter;
    do
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                        &command,
                                        &parameter);
        if (parameter.key == NULL || parameter.value == NULL)
        {
            break;
        }
        else if (strcmp(parameter.key, "datetime") == 0)
        {
            calibration->dateTime = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "offset") == 0)
        {
            calibration->userOffset = strtol(parameter.value, NULL, 10);
        }
        else if (strcmp(parameter.key, "slope") == 0)
        {
            calibration->userSlope = strtol(parameter.value, NULL, 10);
        }
        else if (parameter.key[0] != 'c'
                 && parameter.key[0] != 'x'
                 && parameter.key[0] != 'n')
        {
            continue;
        }

        int32_t index = strtol(&parameter.key[1], NULL, 10);

        if (parameter.key[0] == 'c'
            && index < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX)
        {
            calibration->c[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'x'
                 && index < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX)
        {
            calibration->x[index] = strtod(parameter.value, NULL);
        }
        else if (parameter.key[0] == 'n'
                 && index < RBRINSTRUMENTGEN4_CALIBRATION_N_COEFFICIENT_MAX)
        {
            /* GEN4TODO: Get the dependents by reference */
            /*
            snprintf(calibration->n[index],
                     sizeof(calibration->n[index]),
                     "%s",
                     parameter.value);
            */
        }

    } while (true);

    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Calibration *calibration)
{
    //GEN4 todo: add logic.
    char calibrationDateTime[RBRINSTRUMENTGEN4_SCHEDULE_TIME_LEN + 1];
    RBRInstrumentGen4DateTime_toScheduleTime(calibration->dateTime,
                                         calibrationDateTime);

    const char *calibrationCommand = "calibration %s datetime = %s, %c%d = %g";

    RBRInstrumentGen4Channel *parent = (RBRInstrumentGen4Channel *)(calibration->parent);
    RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                "calibration %s datetime=%s, useroffset=%d, userslope=%d",
                                parent->label,
                                calibrationDateTime,
                                (double) calibration->userOffset,
                                (double) calibration->userSlope));

    for (int32_t c = 0;
         c < RBRINSTRUMENTGEN4_CALIBRATION_C_COEFFICIENT_MAX
         && !isnan(calibration->c[c]);
         ++c)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       parent->label,
                                       calibrationDateTime,
                                       'c',
                                       c,
                                       (double) calibration->c[c]));
    }
    for (int32_t x = 0;
         x < RBRINSTRUMENTGEN4_CALIBRATION_X_COEFFICIENT_MAX
         && !isnan(calibration->x[x]);
         ++x)
    {
        RBR_TRY(RBRInstrumentGen4_converse(instrument,
                                       calibrationCommand,
                                       parent->label,
                                       calibrationDateTime,
                                       'x',
                                       x,
                                       (double) calibration->x[x]));
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
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

const char *RBRInstrumentGen4ChannelGainMode_name(
    RBRInstrumentGen4ChannelGainMode mode)
{
    switch (mode)
    {
    case RBRINSTRUMENTGEN4_GAIN_NONE:
        return "none";
    case RBRINSTRUMENTGEN4_GAIN_FIXED:
        return "manual";
    case RBRINSTRUMENTGEN4_GAIN_AUTO:
        return "auto";
    case RBRINSTRUMENTGEN4_GAIN_COUNT:
        return "gain mode count";
    case RBRINSTRUMENTGEN4_UNKNOWN_GAIN:
    default:
        return "unknown gain mode";
    }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4Channel *channel){
    //GEN4 todo: add logic
    (void)instrument;
    (void)channel;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPool(RBRInstrumentGen4 *instrument,
                                             RBRInstrumentGen4ChannelPool *channelPool)
{
    //GEN4 todo: add logic.
    //populate all channel instances, which included the calbration instances.
    (void)instrument;
    (void)channelPool;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Settings_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings){
        (void)instrument;
        (void)settings;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Settings_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings){
        (void)instrument;
        (void)settings;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

#if 0
RBRInstrumentGen4Error RBRInstrumentGen4_getSensorParameter(
    RBRInstrumentGen4 *instrument,
    char *channelLabel,
    RBRInstrumentGen4SensorParameter *parameter)
{
    memset(parameter->value, 0, sizeof(parameter->value));

    RBRInstrumentGen4Error err;
    /* Logger2 returns “E0501 item is not configured” when the requested
     * parameter doesn't exist, so we can't wrap the conversation in RBR_TRY
     * because we need to suppress that error. */
    err = RBRInstrumentGen4_converse(instrument,
                                 "sensor %s %s",
                                 channelLabel,
                                 parameter->key);

    if (instrument->generation == RBRINSTRUMENTGEN4_LOGGER2
        && err == RBRINSTRUMENTGEN4_HARDWARE_ERROR
        && (instrument->response.error ==
            RBRINSTRUMENTGEN4_HARDWARE_ERROR_ITEM_IS_NOT_CONFIGURED))
    {
        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "n/a");
        instrument->response.type = RBRINSTRUMENTGEN4_RESPONSE_INFO;
        return RBRINSTRUMENTGEN4_SUCCESS;
    }
    else if (err != RBRINSTRUMENTGEN4_SUCCESS)
    {
        return err;
    }

    char *command = NULL;
    RBRInstrumentGen4ResponseParameter responseParameter;
    while (true)
    {
        RBRInstrumentGen4_parseResponse(instrument,
                                    &command,
                                    &responseParameter);

        if (responseParameter.key == NULL)
        {
            break;
        }

        snprintf(parameter->key,
                 sizeof(parameter->key),
                 "%s",
                 responseParameter.key);

        snprintf(parameter->value,
                 sizeof(parameter->value),
                 "%s",
                 responseParameter.value);
    }

    return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_getSensorParameters(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    int32_t *size,
    RBRInstrumentGen4SensorParameter *parameters)
{
    (void)instrument;
    (void)channelLabel;
    (void)size;
    (void)parameters;
    return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setSensorParameter(
    RBRInstrumentGen4 *instrument,
    const char *channelLabel,
    const RBRInstrumentGen4SensorParameter *parameter)
{
    return RBRInstrumentGen4_converse(instrument,
                                  "sensor %s %s=%s",
                                  channelLabel,
                                  parameter->key,
                                  parameter->value);
}
#endif

const char *RBRInstrumentGen4UvledCommand_name(RBRInstrumentGen4UvledCommand uvledCommand)
{
            switch(uvledCommand){
            case RBRINSTRUMENTGEN4_UVLED_ACTIVATE:
                return "activate";
            case RBRINSTRUMENTGEN4_UVLED_DEACTIVATE:
                return "deactivate";
            case RBRINSTRUMENTGEN4_UVLED_STATUS:
                return "status";
            case RBRINSTRUMENTGEN4_UVLED_COUNT:
                return "uvled command count";
            case RBRINSTRUMENTGEN4_UNKNOWN_UVLED:
            default:
                return "unknown uvled command";
            }
}

RBRInstrumentGen4Error RBRInstrumentGen4_getUvled(RBRInstrumentGen4 *instrument, RBRInstrumentGen4Uvled *uvled){
        (void)instrument;
        (void)uvled;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4Uvled_setUvled(RBRInstrumentGen4 *instrument, const RBRInstrumentGen4Uvled *uvled){
        (void)instrument;
        (void)uvled;
        return RBRINSTRUMENTGEN4_SUCCESS;
}


RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(RBRInstrumentGen4 *instrument,
                                                  RBRInstrumentGen4ChannelPool *channelPool,
                                                  RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)channelPool;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(RBRInstrumentGen4 *instrument, 
                                                  RBRInstrumentGen4Group *group){
        (void)instrument;
        (void)group;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(
    RBRInstrumentGen4 *instrument,
    const char *newGroupLabel,
    RBRInstrumentGen4GroupPool *groupPool,
    RBRInstrumentGen4Group **newGroup){
        (void)instrument;
        (void)newGroupLabel;
        (void)groupPool;
        (void)newGroup;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4Group *groupToDelete){
        (void)instrument;
        (void)groupToDelete;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4GroupPool *groupPool){
    (void)instrument;
    (void)groupPool;                                                            
    return RBRINSTRUMENTGEN4_SUCCESS;
}                                                    
RBRInstrumentGen4Error RBRInstrumentGen4_getGroupPool(RBRInstrumentGen4 *instrument, RBRInstrumentGen4GroupPool *groupPool){
        (void)instrument;
        (void)groupPool;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(
    RBRInstrumentGen4 *instrument, 
    RBRInstrumentGen4SchedulePool *schedulePool,
    RBRInstrumentGen4Config *config){
        (void)instrument;
        (void)schedulePool;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(RBRInstrumentGen4 *instrument, 
                                                   RBRInstrumentGen4Config *config)
{
        (void)instrument;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_getConfigPool(RBRInstrumentGen4 *instrument, RBRInstrumentGen4ConfigPool *configPool){
        (void)instrument;
        (void)configPool;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(
    RBRInstrumentGen4 *instrument, 
    const char *newConfigLabel,
    RBRInstrumentGen4ConfigPool *configPool,
    RBRInstrumentGen4Config **newConfig)
{
        (void)instrument;
        (void)newConfigLabel;
        (void)configPool;
        (void)newConfig;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(RBRInstrumentGen4 *instrument, 
                                                    RBRInstrumentGen4Config *config)
{
        (void)instrument;
        (void)config;
        return RBRINSTRUMENTGEN4_SUCCESS;
}

RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(RBRInstrumentGen4 *instrument,
                                                    RBRInstrumentGen4ConfigPool *configPool){
        (void)instrument;
        (void)configPool;
        return RBRINSTRUMENTGEN4_SUCCESS;                                                
}
