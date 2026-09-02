/**
 * \file RBRInstrumentGen4Configuration.h
 *
 * \brief Instrument commands and structures pertaining to instrument
 * configuration information and calibration.
 *
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 *
 * \copyright
 * Copyright (c) 2018 RBR Ltd.
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBRBR_RBRINSTRUMENTGEN4CONFIGURATION_H
#define LIBRBR_RBRINSTRUMENTGEN4CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RBRInstrumentGen4.h"

/** \brief The maximum number of schedules count. */
#define RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX 16

/** \brief The maximum number of permissionlist count. */
#define RBRINSTRUMENTGEN4_PERMISSION_COUNT_MAX 14

/** \brief The maximum schedule period in milliseconds. */
#define RBRINSTRUMENTGEN4_SAMPLING_PERIOD_MAX 86400000

/** \brief The maximum regime boundary in dbar. */
#define RBRINSTRUMENTGEN4_REGIME_BOUNDARY_MAX 65535

/** \brief The maximum regime bin size in dbar. */
#define RBRINSTRUMENTGEN4_REGIME_BINSIZE_MAX 6553.5

/** \brief The maximum sampling period within a regime. */
#define RBRINSTRUMENTGEN4_REGIME_SAMPLING_PERIOD_MAX 65000

/**
 * \brief The maximum number of coefficients in a calibration group.
 *
 * All three groups share one pool of this size on the instrument, so the
 * three counts sum to no more than this.
 *
 * \see RBRInstrumentGen4Calibration.a
 * \see RBRInstrumentGen4Calibration.b
 * \see RBRInstrumentGen4Calibration.m
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX 14

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 * \see RBRInstrumentGen4Calibration.equation
 */
#define RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX 32

/** \brief The minimum input timeout. */
#define RBRINSTRUMENTGEN4_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRINSTRUMENTGEN4_INPUT_TIMEOUT_MAX 240000

/** 
 * \brief The maximum number of configs count.
 * \see RBRInstrumentGen4ConfigPool.pool
 */
#define RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX 16

/**
 * \brief The maximum number of groups count.
 * \see RBRInstrumentGen4GroupPool.pool
 */
#define RBRINSTRUMENTGEN4_GROUP_COUNT_MAX 16

/** \brief The maximum number of characters in a bus address.
 * The bus address is from 0 to 255, with some reserved addresses.
 */
#define RBRINSTRUMENTGEN4_BUS_ADDRESS_MAX 3

/** \brief The maximum number of fast periods. */
#define RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX 4

/**
 * \brief The maximum number of nodes.
 * \see RBRInstrumentGen4NodePool.pool
 */
#define RBRINSTRUMENTGEN4_NODE_COUNT_MAX 12

/**
 * \brief The maximum number of ports.
 * \see RBRInstrumentGen4Node.portList
 */
#define RBRINSTRUMENTGEN4_PORT_COUNT_MAX 16

/**
 * \brief The maximum number of devices.
 * \see RBRInstrumentGen4Port.deviceList
 */
#define RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX 16

/**
 * \brief `node <node_label>` command parameters.
 *
 * A node is a front-end PCBA, plus the `self` node standing for the main CPU
 * board. Nodes are the top of the instrument's configuration hierarchy: a node
 * carries ports, a port carries devices, and a device exposes channels.
 *
 * \see RBRInstrumentGen4NodePool
 * \see RBRInstrumentGen4_getNode()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4Node
{
    /**
     * \brief Node label.
     *
     * Set by the caller to select the node to read; see
     * RBRInstrumentGen4_getNode().
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the PCBA implementing the node. */
    char pcba[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of ports on the node. */
    int32_t portCount;

    /** \brief The labels of the ports on the node. */
    char portList[RBRINSTRUMENTGEN4_PORT_COUNT_MAX]
                 [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The firmware version running on the node. */
    char fwVersion[RBRINSTRUMENTGEN4_ID_VERSION_MAX + 1];

    /** \brief The node firmware version in semantic-version form. */
    char semver[RBRINSTRUMENTGEN4_ID_SEMVER_MAX + 1];

    /**
     * \brief The firmware type running on the node.
     *
     * Zero when the node runs no firmware of its own, which it reports as
     * `na`.
     */
    int32_t fwType;

    /** \brief The time in milliseconds the node takes to power up. */
    int32_t powerUpTime;

    /**
     * \brief The time in milliseconds to wait after powering the node up
     * before powering anything beneath it.
     */
    int32_t inrushOffsetTime;
} RBRInstrumentGen4Node;

/**
 * \brief Populate the parameters of a node.
 *
 * The caller sets RBRInstrumentGen4Node.label to select the node to read.
 *
 * \note Issues the `node <node_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] node the node to read
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the node is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getNodePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getNode(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Node *node);

/**
 * \brief `node` command parameters.
 *
 * \see RBRInstrumentGen4_getNodePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4NodePool
{
    /**
     * \brief The number of nodes on the instrument.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_NODE_COUNT_MAX)` to avoid an
     * out-of-bounds error when accessing #pool if 
     * #count > #RBRINSTRUMENTGEN4_NODE_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of nodes. */
    RBRInstrumentGen4Node pool[RBRINSTRUMENTGEN4_NODE_COUNT_MAX];
} RBRInstrumentGen4NodePool;

/**
 * \brief Populate the pool of the instrument's nodes.
 *
 * Only the labels are reported; read the rest of a node's parameters with
 * RBRInstrumentGen4_getNode().
 *
 * \note Issues the `node` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] nodePool the populated pool of nodes
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the nodes are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getNode()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getNodePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4NodePool *nodePool);

/**
 * \brief The classes of port.
 *
 * \see RBRInstrumentGen4Port.portClass
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRInstrumentGen4PortClass
{
    /** A direct ADC connection with no bus; typical of on-board sensors. */
    RBRINSTRUMENTGEN4_PORT_CLASS_VIRTUAL,
    /** A bus-attached port, which speaks one of the port protocols. */
    RBRINSTRUMENTGEN4_PORT_CLASS_SERIAL,

    /** The number of specific port classes. */
    RBRINSTRUMENTGEN4_PORT_CLASS_COUNT,
    /** An unknown or unrecognized port class. */
    RBRINSTRUMENTGEN4_UNKNOWN_PORT_CLASS
} RBRInstrumentGen4PortClass;

/**
 * \brief Get a human-readable string name for a port class.
 *
 * \param [in] portClass the port class
 * \return a string name for the port class
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4PortClass_name(
    RBRInstrumentGen4PortClass portClass);

/**
 * \brief The protocols a port can speak.
 *
 * A `virtual` port speaks none of these. Consult
 * RBRInstrumentGen4Port.availableProtocols for the protocols a given port is
 * capable of.
 *
 * \see RBRInstrumentGen4Port
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRInstrumentGen4PortProtocol
{
    /** An unrecognized protocol, or none being spoken. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_NONE         =      0,
    /** Pressure. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_PRESSURE = 1 << 0,
    /** RBR serial. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRSERIAL    = 1 << 1,
    /** RBR modem. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMODEM     = 1 << 2,
    /** RBR multidrop. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMULTIDROP = 1 << 3,
    /** Corresponds to the largest port protocol enum value. */
    RBRINSTRUMENTGEN4_PORT_PROTOCOL_MAX
        = RBRINSTRUMENTGEN4_PORT_PROTOCOL_RBRMULTIDROP
} RBRInstrumentGen4PortProtocol;

/**
 * \brief Get a human-readable string name for a port protocol.
 *
 * \param [in] protocol the port protocol
 * \return a string name for the port protocol
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4PortProtocol_name(
    RBRInstrumentGen4PortProtocol protocol);

/**
 * \brief `port <port_label>` command parameters.
 *
 * A port is an attachment point on a node to which devices attach.
 *
 * \see RBRInstrumentGen4PortPool
 * \see RBRInstrumentGen4_getPort()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4Port
{
    /**
     * \brief Port label.
     *
     * Set by the caller to select the port to read; see
     * RBRInstrumentGen4_getPort().
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the node the port belongs to. */
    char node[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The class of the port. */
    RBRInstrumentGen4PortClass portClass;

    /** \brief The protocol currently selected on the port. */
    RBRInstrumentGen4PortProtocol protocol;

    /**
     * \brief Protocols the port is capable of speaking.
     *
     * Treated as a bit field representation of available protocols as defined
     * by RBRInstrumentGen4PortProtocol. For details, consult
     * [Working with Bit Fields](bitfields.md).
     */
    RBRInstrumentGen4PortProtocol availableProtocols;

    /**
     * \brief The baud rate of the port.
     *
     * Zero for a `virtual` port.
     */
    int32_t baudRate;

    /** \brief The number of devices attached to the port. */
    int32_t deviceCount;

    /** \brief The labels of the devices attached to the port. */
    char deviceList[RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX]
                   [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The time in milliseconds to bring power to the port. */
    int32_t powerUpTime;
} RBRInstrumentGen4Port;

/**
 * \brief Populate the parameters of a port.
 *
 * The caller sets RBRInstrumentGen4Port.label to select the port to read.
 *
 * \note Issues the `port <port_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] port the port to read
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the port is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getPortPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPort(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Port *port);

/**
 * \brief `port` command parameters.
 *
 * \see RBRInstrumentGen4_getPortPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4PortPool
{
    /**
     * \brief The number of ports across all nodes.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_PORT_COUNT_MAX)` to avoid an
     * out-of-bounds error when accessing #pool if 
     * #count > #RBRINSTRUMENTGEN4_PORT_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of ports. */
    RBRInstrumentGen4Port pool[RBRINSTRUMENTGEN4_PORT_COUNT_MAX];
} RBRInstrumentGen4PortPool;

/**
 * \brief Populate the pool of the instrument's ports.
 *
 * Only the labels are reported; read the rest of a port's parameters with
 * RBRInstrumentGen4_getPort().
 *
 * \note Issues the `port` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] portPool the populated pool of ports
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the ports are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getPort()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getPortPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4PortPool *portPool);

/**
 * \brief The classes of device.
 *
 * \see RBRInstrumentGen4Device.deviceClass
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRInstrumentGen4DeviceClass
{
    /** A device which measures. */
    RBRINSTRUMENTGEN4_DEVICE_CLASS_SENSOR,
    /** A device which acts, such as a valve or a UV LED. */
    RBRINSTRUMENTGEN4_DEVICE_CLASS_ACTUATOR,
    /** A device which raises asynchronous events. */
    RBRINSTRUMENTGEN4_DEVICE_CLASS_EVENTGEN,
    /** A device which carries other devices on a multidrop bus. */
    RBRINSTRUMENTGEN4_DEVICE_CLASS_MODEM,

    /** The number of specific device classes. */
    RBRINSTRUMENTGEN4_DEVICE_CLASS_COUNT,
    /** An unknown or unrecognized device class. */
    RBRINSTRUMENTGEN4_UNKNOWN_DEVICE_CLASS
} RBRInstrumentGen4DeviceClass;

/**
 * \brief Get a human-readable string name for a device class.
 *
 * \param [in] deviceClass the device class
 * \return a string name for the device class
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4DeviceClass_name(
    RBRInstrumentGen4DeviceClass deviceClass);

/**
 * \brief `device <device_label>` command parameters.
 *
 * A device is a logical sensor, actuator, event source, or modem attached to a
 * port. Devices are produced by discovery: there is no command to create one.
 *
 * \see RBRInstrumentGen4DevicePool
 * \see RBRInstrumentGen4_getDevice()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4Device
{
    /**
     * \brief Device label.
     *
     * Set by the caller to select the device to read; see
     * RBRInstrumentGen4_getDevice().
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the port the device is attached to. */
    char port[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The class of the device. */
    RBRInstrumentGen4DeviceClass deviceClass;

    /**
     * \brief Device serial number.
     *
     * Zero when the instrument has no serial number recorded for the device,
     * which it reports as `na`.
     */
    int32_t sn;

    /**
     * \brief Device part number.
     *
     * Reported as `na` when unrecorded.
     */
    char pn[RBRINSTRUMENTGEN4_PART_NUMBER_MAX + 1];

    /** \brief The firmware version running on the device. */
    char fwVersion[RBRINSTRUMENTGEN4_ID_VERSION_MAX + 1];

    /**
     * \brief The firmware type running on the device.
     *
     * Zero when the device runs no firmware of its own, which it reports as
     * `na`.
     */
    int32_t fwType;

    /** \brief The generic name of the kind of device installed. */
    char name[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of channels the device exposes. */
    int32_t channelCount;

    /**
     * \brief The labels of the channels the device exposes.
     *
     * A device can name a channel which the `channel` command does not
     * enumerate and will not accept, so a label found here is not
     * necessarily readable with RBRInstrumentGen4_getChannel().
     */
    char channelList[RBRINSTRUMENTGEN4_CHANNEL_MAX]
                    [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief Whether the device is protected from being overridden by a
     * subsequent device discovery.
     */
    bool lock;

    /** \brief The time in milliseconds the device takes to power up. */
    int32_t powerUpTime;

    /** \brief The time in milliseconds the device needs between samples. */
    int32_t coolDownTime;

    /** \brief The time in milliseconds the device takes to power down. */
    int32_t powerDownTime;

    /**
     * \brief The time in milliseconds to wait after powering the device up
     * before drawing on it.
     */
    int32_t inrushOffsetTime;
} RBRInstrumentGen4Device;

/**
 * \brief Populate the parameters of a device.
 *
 * The caller sets RBRInstrumentGen4Device.label to select the device to read.
 *
 * \note Issues the `device <device_label> <param1> <param2> ...` command
 * \note This getter is special: the `device <device_label>` command has a
 * hidden `lock` parameter which does not appear unless queried by name, so this
 * getter explicitly requests *every* parameter of the command by name. This 
 * results in a larger command string than most getters, and therefore it may
 * take slightly longer to converse. 
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] device the device to read
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the device is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getDevice(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Device *device);

/**
 * \brief `device` command parameters.
 *
 * \see RBRInstrumentGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRInstrumentGen4DevicePool
{
    /**
     * \brief The number of devices across all ports.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX)` to avoid
     * an out-of-bounds error when accessing #pool if 
     * #count > #RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of devices. */
    RBRInstrumentGen4Device pool[RBRINSTRUMENTGEN4_DEVICE_COUNT_MAX];
} RBRInstrumentGen4DevicePool;

/**
 * \brief Populate the pool of the instrument's devices.
 *
 * Only the labels are reported; read the rest of a device's parameters with
 * RBRInstrumentGen4_getDevice().
 *
 * \note Issues the `device` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] devicePool the populated pool of devices
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the devices are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getDevice()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getDevicePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DevicePool *devicePool);

/**
 * \brief Sweep every port and repopulate the devices attached to them.
 *
 * Discovery reports every device present after the sweep, not only the ones it
 * has just added, so it fills the same pool RBRInstrumentGen4_getDevicePool()
 * does. Both report nothing but the labels; read a device's parameters with
 * RBRInstrumentGen4_getDevice().
 *
 * \note Issues the `device discover` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] devicePool the labels of the devices present after the sweep
 * \return #RBRINSTRUMENTGEN4_SUCCESS when discovery completes
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the instrument refuses
 * \see RBRInstrumentGen4_getDevice()
 * \see RBRInstrumentGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_discoverDevices(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4DevicePool *devicePool);

/**
 * \brief `calibration <channel_label>` command parameters.
 *
 * The number and kind of coefficients depend on the channel's equation.
 * Coefficients the equation does not use are absent from the response and the
 * instrument rejects them, so a group's count bounds what is present.
 *
 * \see RBRInstrumentGen4Channel
 * \see RBRInstrumentGen4_getCalibration()
 * \see RBRInstrumentGen4_setCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
typedef struct RBRInstrumentGen4Calibration
{
    /**
     * \brief The label of the channel the calibration belongs to.
     *
     * Set by the caller to select the calibration to read; see
     * RBRInstrumentGen4_getCalibration(). Calibrations are one to one with
     * channels and cannot be created or deleted.
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The formula used to convert raw readings to physical units.
     *
     * E.g. `temperature`, `linear`, `deri_depth`.
     */
    const char equation[RBRINSTRUMENTGEN4_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The date and time of the calibration.
     *
     * `20000101000000` means the channel has never been calibrated. The
     * instrument restamps this with the current time when coefficients change
     * and no date is sent with them.
     */
    RBRInstrumentGen4DateTime dateTime;

    /**
     * \brief A linear offset applied to the final value.
     *
     * Not part of the calibration proper; provided for a rough field
     * correction when a recalibration is not possible.
     */
    float userOffset;

    /**
     * \brief A linear slope applied to the final value.
     *
     * Not part of the calibration proper; provided for a rough field
     * correction when a recalibration is not possible.
     */
    float userSlope;

    /** \brief The number of a coefficients the equation uses. */
    int32_t aCount;

    /** \brief The a coefficients, which any user may change. */
    float a[RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX];

    /** \brief The number of b coefficients the equation uses. */
    int32_t bCount;

    /**
     * \brief The b coefficients.
     *
     * \warning Intended to be changed by RBR or an expert user only.
     */
    float b[RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX];

    /** \brief The number of m references the equation uses. */
    int32_t mCount;

    /**
     * \brief The cross-channel references the equation takes as inputs.
     *
     * Each is the label of another channel, a `param_`-prefixed name from the
     * `parameters` command, or `internal`. An entry the equation leaves unused
     * is empty, and is sent as `none`. Whether a label names something the
     * equation can use is for the instrument to decide.
     *
     * \see RBRInstrumentGen4_getParameters()
     */
    char m[RBRINSTRUMENTGEN4_CALIBRATION_COEFFICIENT_MAX]
          [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
} RBRInstrumentGen4Calibration;

/** \brief An internal module identifier. */
typedef uint8_t RBRInstrumentGen4ModuleAddress;

/**
 * \brief Whether a channel carries a measurement or an instrument housekeeping
 * value.
 *
 * \see RBRInstrumentGen4Channel
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef enum RBRInstrumentGen4ChannelNature
{
    /** The channel measures a physical parameter. */
    RBRINSTRUMENTGEN4_CHANNEL_NATURE_SCIENTIFIC,
    /** The channel reports an instrument housekeeping value. */
    RBRINSTRUMENTGEN4_CHANNEL_NATURE_SYSTEM,
    /** The number of specific channel natures. */
    RBRINSTRUMENTGEN4_CHANNEL_NATURE_COUNT,
    /** An unknown or unrecognized channel nature. */
    RBRINSTRUMENTGEN4_UNKNOWN_CHANNEL_NATURE
} RBRInstrumentGen4ChannelNature;

/**
 * \brief Get a human-readable string name for a channel nature.
 *
 * \param [in] nature the channel nature
 * \return a string name for the channel nature
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ChannelNature_name(
    RBRInstrumentGen4ChannelNature nature);

/**
 * \brief Instrument `channel <channel_label>` command parameters.
 *
 * \see RBRInstrumentGen4ChannelPool
 * \see RBRInstrumentGen4_getChannel()
 * \see RBRInstrumentGen4_setChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef struct RBRInstrumentGen4Channel
{
    /**
     * \brief The channel's label.
     *
     * Set by the caller to select the channel to read; see
     * RBRInstrumentGen4_getChannel().
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief A short, pre-defined generic name for the installed channel.
     *
     * E.g. `temp006`, `pres003`, `dpth001`.
     */
    const char type[RBRINSTRUMENTGEN4_CHANNEL_TYPE_MAX + 1];

    /** \brief Settling time in milliseconds; zero on a derived channel. */
    const RBRInstrumentGen4Period settlingTime;

    /** \brief Measuring time in milliseconds; zero on a derived channel. */
    const RBRInstrumentGen4Period measuringTime;

    /** \brief Read-out time in milliseconds; zero on a derived channel. */
    const RBRInstrumentGen4Period readOutTime;

    /**
     * \brief The unit in which processed data is reported.
     *
     * The only parameter of the command a caller may change.
     */
    char userUnits[RBRINSTRUMENTGEN4_CHANNEL_UNIT_MAX + 1];

    /** \brief The number of groups this channel belongs to. */
    const int32_t groupCount;

    /**
     * \brief The labels of the groups this channel belongs to.
     *
     * Membership is changed through the `group` command, not here.
     * \see RBRInstrumentGen4_setGroup()
     */
    const char groupList[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX]
                        [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief Whether the channel measures or reports housekeeping. */
    const RBRInstrumentGen4ChannelNature nature;

    /**
     * \brief Whether the channel is computed from other channels rather than
     * measured.
     */
    const bool derived;

    /**
     * \brief The label of the node the channel is reached through.
     *
     * `self` for a channel of the instrument itself, and empty for a derived
     * channel, which the instrument reports as `na`.
     */
    const char node[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The label of the port the channel is reached through.
     *
     * Empty for a derived channel, which the instrument reports as `na`.
     */
    const char port[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The label of the device the channel belongs to.
     *
     * Empty for a derived channel, which the instrument reports as `na`.
     */
    const char device[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
} RBRInstrumentGen4Channel;

/**
 * \brief Instrument `channel` command parameters.
 *
 * \see RBRInstrumentGen4_getChannelPool()
 * \see RBRInstrumentGen4_getChannelPoolByNature()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef struct RBRInstrumentGen4ChannelPool
{
    /**
     * \brief The number of channels reported.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_CHANNEL_MAX)` to avoid an
     * out-of-bounds error when accessing #pool if 
     * #count > #RBRINSTRUMENTGEN4_CHANNEL_MAX.
     */
    int32_t count;

    /**
     * \brief The channels reported.
     *
     * Discovery reports nothing but the labels; read a channel's parameters
     * with RBRInstrumentGen4_getChannel().
     */
    RBRInstrumentGen4Channel pool[RBRINSTRUMENTGEN4_CHANNEL_MAX];
} RBRInstrumentGen4ChannelPool;

/**
 * \brief Populate the parameters of a channel.
 *
 * The caller sets RBRInstrumentGen4Channel.label to select the channel to
 * read.
 *
 * \note Issues the `channel <channel_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] channel the channel to read, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the channel is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the channel does not exist
 * \see RBRInstrumentGen4_getChannelPool()
 * \see RBRInstrumentGen4_setChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannel(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Channel *channel);

/**
 * \brief Update a channel's user units.
 *
 * RBRInstrumentGen4Channel.userUnits is the only parameter of the command a
 * caller may change; every other field of the structure is `const`. Read the
 * channel with RBRInstrumentGen4_getChannel(), change the units, and write the
 * structure back.
 *
 * \note Issues the `channel <channel_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] channel the channel to write, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the channel is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the channel cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the units are empty
 * \see RBRInstrumentGen4_getChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setChannel(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Channel *channel);

/**
 * \brief Populate the pool of channels configured on the instrument.
 *
 * Reports nothing but the labels; read a channel's parameters with
 * RBRInstrumentGen4_getChannel().
 *
 * \note Issues the `channel` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] channelPool the labels of the channels present
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the pool is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the channel pool cannot be read
 * \see RBRInstrumentGen4_getChannelPoolByNature()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelPool *channelPool);

/**
 * \brief Populate the pool of channels of one nature.
 *
 * Reports nothing but the labels; read a channel's parameters with
 * RBRInstrumentGen4_getChannel().
 *
 * \note Issues the `channel scientific` or `channel system` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] nature the nature of the channels to report
 * \param [out] channelPool the labels of the channels present
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the pool is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR if the channel pool cannot be read
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the nature is not
 *                                                    one the command accepts
 * \see RBRInstrumentGen4_getChannelPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getChannelPoolByNature(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ChannelNature nature,
    RBRInstrumentGen4ChannelPool *channelPool);

/**
 * \brief Read a channel's calibration.
 *
 * The caller sets RBRInstrumentGen4Calibration.label to select the channel.
 *
 * \note Issues the `calibration <channel_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] calibration the calibration to read, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the calibration is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the channel does not exist
 * \see RBRInstrumentGen4_setCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getCalibration(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Calibration *calibration);

/**
 * \brief Update a channel's calibration.
 *
 * Sends the date, the offset and slope, and every a, b, and m coefficient the
 * equation uses. Read the calibration with RBRInstrumentGen4_getCalibration(),
 * change what you need, and write the structure back: the counts read there
 * are what bounds the coefficients sent.
 *
 * The equation is `const` and never sent; the instrument rejects a write to it.
 *
 * \warning Hardware errors may occur if the instrument is logging, a
 *          coefficient is out of range for the equation, or an m reference
 *          does not name something the equation can use.
 *
 * \param [in] instrument the instrument connection
 * \param [in] calibration the calibration to write, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the calibration is successfully
 *                                    written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the calibration cannot be
 *                                           changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when a coefficient count
 *                                                    is out of range
 * \see RBRInstrumentGen4_getCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setCalibration(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Calibration *calibration);


/** 
 * \brief Instrument `settings` command parameters.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_getSettings()
 * \see RBRInstrumentGen4_setSettings()
 */
typedef struct RBRInstrumentGen4Settings
{
    /**
     * \brief Whether the instrument returns the “Ready:” prompt following a
     * response. The as-shipped default value is on.
     */
    bool prompt;

    /**
     * \brief Whether the instrument returns a response from a create or
     * set/modify operation to verify the new state. The as-shipped default
     * value is on.
     * A response will always be sent when a parameter value is simply
     * requested.
     */
    bool confirmation;

    /**
     * \brief The delay in milliseconds between the completion of a poll and
     * the removal of sensor power. The as-shipped default value is 8000.
     */
    RBRInstrumentGen4Period pollPowerOffDelay;
} RBRInstrumentGen4Settings;

/**
 * \brief Get miscellaneous logger settings
 * \note Issues the instrument `settings` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] settings the logger settings
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_setSettings()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSettings(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Settings *settings);

/**
 * \brief Set the miscellaneous logger settings.
 * \note Issues the instrument `settings` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] settings the values for the settings in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the setting is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the settings cannot be changed
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the power-off delay
 *                                                    is negative
 * \warning The library expects both \a prompt and \a confirmation to be on.
 *          With \a confirmation off the instrument answers a set with nothing
 *          at all, and every later setter blocks until the command timeout.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 * \see RBRInstrumentGen4_getSettings()
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSettings(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Settings *settings);

/** 
 * \brief `parameters` command parameters.
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_getParameters()
 * \see RBRInstrumentGen4_setParameters()
 */
typedef struct RBRInstrumentGen4Parameters
{
    /**
     * \brief the temperature coefficient used to correct the derived channel 
     * for specific conductivity to 25°C. Its value depends on the ionic 
     * composition of the water being monitored, and should be set to an 
     * appropriate value for best results. 
     * A typical range of values is 0.0191 to 0.0214, with the lower end 
     * suitable for KCl solutions and the upper end for NaCl solutions.
     */
    float specCondTempCo;
    /** \brief the height above the seabed in metres at which the logger 
     * is deployed. This is a user-entered parameter which is required by 
     * host software to calculate statistics and parameters for 
     * wave analysis. Can be ignored if not used.*/
    float altitude;
    /** \brief below are default parameter values, to be used when the logger does 
     * not have a channel which measures the named parameter, but one or more 
     * cross-channel calibration equations requires it as an input.
     * temperature in °C, default value 15.0*/
    float temperature;
    /** \brief absolute pressure in dbar, default value 10.132501 (1 standard atmosphere)*/
    float pressure;
    /** \brief atmospheric pressure in dbar, default value 10.132501*/
    float atmosphere;
    /** \brief water density in g/cm3, default value 1.026021*/
    float density;
    /** \brief salinity in PSU, default value 35*/
    float salinity;
    /** \brief avgSoundSpeed in m/s, default value 1506.8*/
    float avgSoundSpeed;
} RBRInstrumentGen4Parameters;

/**
 * \brief Get parameters which may be required when computing calibrated output.
 * \note Issues the instrument `parameters` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] parameters the parameters in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_setParameters
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getParameters(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Parameters *parameters);

/**
 * \brief Set parameters which may be required when computing calibrated output.
 * \note Issues the instrument `parameters` command.
 *
 * \warning Hardware errors may occur if the instrument is logging.
 *
 * \param [in] instrument the instrument connection
 * \param [in] parameters the values for the parameters in the logger
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the parameters are successfully
 *                                    written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the parameters cannot be
 *                                           changed
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 * \see RBRInstrumentGen4_getParameters
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setParameters(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Parameters *parameters);

/**
 * \brief `group <group_label>` command parameters.
 *
 * \see RBRInstrumentGen4GroupPool
 * \see RBRInstrumentGen4_getGroup()
 * \see RBRInstrumentGen4_setGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
typedef struct RBRInstrumentGen4Group
{
    /**
     * \brief The group's label.
     *
     * Set by the caller to select the group to read.
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of channels in the group. */
    int32_t channelCount;

    /** \brief The labels of the channels in the group. */
    char channelList[RBRINSTRUMENTGEN4_CHANNEL_MAX]
                    [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The number of schedules using the group.
     *
     * \readonly
     */
    const int32_t scheduleCount;

    /**
     * \brief The labels of the schedules using the group.
     *
     * \readonly
     * \see RBRInstrumentGen4_setSchedule()
     */
    const char scheduleList[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX]
                           [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
} RBRInstrumentGen4Group;

/**
 * \brief Populate the parameters of a group.
 *
 * The caller sets RBRInstrumentGen4Group.label to select the group to read.
 *
 * \note Issues the `group <group_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] group the group to read, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the group does not exist
 * \see RBRInstrumentGen4_getGroupPool()
 * \see RBRInstrumentGen4_setGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroup(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Group *group);

/**
 * \brief Set the channels in a group.
 *
 * Sends `channellist`, the only writable parameter. A zero
 * RBRInstrumentGen4Group.channelCount sends `none`.
 *
 * \note Issues the `group <group_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] group the group to write
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the group cannot be written
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    the channel count is out
 *                                                    of range, or a channel
 *                                                    label is empty
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the list does not fit
 * \see RBRInstrumentGen4_getGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setGroup(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Group *group);

/**
 * \brief `group` command parameters.
 *
 * \see RBRInstrumentGen4_getGroupPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
typedef struct RBRInstrumentGen4GroupPool
{
    /**
     * \brief The number of groups defined on the instrument.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_GROUP_COUNT_MAX)` to avoid an 
     * out-of-bounds error when accessing #pool if 
     * #maxCount > RBRINSTRUMENTGEN4_GROUP_COUNT_MAX.
     */
    int32_t count;

    /** \brief The maximum number of groups that can exist on the instrument. */
    int32_t maxCount;

    /** \brief The pool of groups. */
    RBRInstrumentGen4Group pool[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX];
} RBRInstrumentGen4GroupPool;

/**
 * \brief Populate the pool of the instrument's groups.
 *
 * Only the labels are reported; read a group's parameters with
 * RBRInstrumentGen4_getGroup().
 *
 * \note Issues the `group` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] groupPool the populated pool of groups
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the groups are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getGroupPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4GroupPool *groupPool);

/**
 * \brief Create an empty group.
 *
 * Add channels with RBRInstrumentGen4_setGroup().
 *
 * \note Issues the `group create <group_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label to give the new group
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully created
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the group cannot be created
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createGroup(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete a group.
 *
 * \note Issues the `group delete <group_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label of the group to delete
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the group is successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the group does not exist
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroup(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete every group.
 *
 * \note Issues the `group delete all` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the groups are successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteGroupAll(
    RBRInstrumentGen4 *instrument);

/**
 * \brief The modes of a schedule.
 *
 * Flags, so one type serves both a schedule's mode and the set of modes the
 * instrument offers. A schedule's mode must be a single flag;
 * RBRInstrumentGen4_setSchedule() rejects any other value.
 *
 * \see RBRInstrumentGen4Schedule.mode
 * \see RBRInstrumentGen4SchedulePool.availableModes
 * \see bitfields.md
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRInstrumentGen4ScheduleMode
{
    /** \brief No mode, and any mode the library does not recognize. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_NONE = 0,
    /** \brief Continuous mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS = 1 << 0,
    /** \brief Average mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_AVERAGE = 1 << 1,
    /** \brief Burst mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_BURST = 1 << 2,
    /** \brief Tide mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_TIDE = 1 << 3,
    /** \brief Wave mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_WAVE = 1 << 4,
    /** \brief Direction-dependent mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_DDSAMPLING = 1 << 5,
    /** \brief Regimes mode. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_REGIMES = 1 << 6,
    /** \brief The greatest mode flag. */
    RBRINSTRUMENTGEN4_SCHEDULE_MODE_MAX =
        RBRINSTRUMENTGEN4_SCHEDULE_MODE_REGIMES
} RBRInstrumentGen4ScheduleMode;

/**
 * \brief Whether a schedule's data is stored in memory.
 *
 * \see RBRInstrumentGen4Schedule.storage
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRInstrumentGen4ScheduleStorage
{
    /** Data for this schedule is not stored in memory. */
    RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_OFF,
    /** Data for this schedule is stored in memory. */
    RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_ON,
    /** The number of specific storage states. */
    RBRINSTRUMENTGEN4_SCHEDULE_STORAGE_COUNT,
    /** The parameter was not reported. */
    RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE
} RBRInstrumentGen4ScheduleStorage;

/**
 * \brief Get a human-readable string name for a storage state.
 *
 * \param [in] storage the storage state
 * \return a string name for the storage state
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ScheduleStorage_name(
    RBRInstrumentGen4ScheduleStorage storage);

/**
 * \brief A schedule's parameters in
 *        #RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS.
 *
 * \see RBRInstrumentGen4Schedule.parameters
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4ScheduleModeContinuous
{
    /** \brief `period`, in milliseconds. */
    RBRInstrumentGen4Period period;
} RBRInstrumentGen4ScheduleModeContinuous;

/**
 * \brief A schedule's parameters in the bursting modes.
 *
 * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_AVERAGE,
 * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_BURST,
 * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_TIDE and
 * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_WAVE take the same parameters and so share
 * one structure.
 *
 * \see RBRInstrumentGen4Schedule.parameters
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4ScheduleModeBursting
{
    /** \brief `period`, in milliseconds. */
    RBRInstrumentGen4Period period;

    /** \brief `measurementperiod`, in milliseconds. */
    RBRInstrumentGen4Period measurementPeriod;

    /** \brief `measurementcount`. */
    int32_t measurementCount;
} RBRInstrumentGen4ScheduleModeBursting;

/**
 * \brief Destinations for a schedule's real-time data.
 *
 * \see RBRInstrumentGen4Schedule.stream
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRInstrumentGen4ScheduleStream
{
    /** Data for this schedule is not streamed in real time. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_OFF,
    /** Data for this schedule is streamed over the USB CDC link. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_USB,
    /** Data for this schedule is streamed over the serial link. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_SERIAL,
    /** The number of specific stream destinations. */
    RBRINSTRUMENTGEN4_SCHEDULE_STREAM_COUNT,
    /** An unknown or unrecognized stream destination. */
    RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STREAM
} RBRInstrumentGen4ScheduleStream;

/**
 * \brief Get a human-readable string name for a stream destination.
 *
 * \param [in] stream the stream destination
 * \return a string name for the stream destination
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ScheduleStream_name(
    RBRInstrumentGen4ScheduleStream stream);

/**
 * \brief `schedule <schedule_label>` command parameters.
 *
 * \see RBRInstrumentGen4SchedulePool
 * \see RBRInstrumentGen4_getSchedule()
 * \see RBRInstrumentGen4_setSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4Schedule
{
    /**
     * \brief The schedule's label.
     *
     * Set by the caller to select the schedule to read.
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of groups the schedule samples. */
    int32_t groupCount;

    /** \brief The labels of the groups the schedule samples. */
    char groupList[RBRINSTRUMENTGEN4_GROUP_COUNT_MAX]
                  [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The number of configurations using the schedule.
     *
     * \readonly
     */
    const int32_t configCount;

    /**
     * \brief The labels of the configurations using the schedule.
     *
     * \readonly
     * \see RBRInstrumentGen4_setConfig()
     */
    const char configList[RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX]
                         [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief Where the schedule's data is streamed in real time. */
    RBRInstrumentGen4ScheduleStream stream;

    /**
     * \brief Whether the schedule's data is stored in memory.
     *
     * Some instrument configurations do not support data storage, in which case
     * this field should be left as #RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE.
     * 
     * \see RBRInstrumentGen4ScheduleStorage
     */
    RBRInstrumentGen4ScheduleStorage storage;

    /** \brief `castdetection`, which applies in every mode. */
    bool castDetection;

    /**
     * \brief The mode the schedule runs in.
     *
     * Exactly one mode flag.
     */
    RBRInstrumentGen4ScheduleMode mode;

    /**
     * \brief The parameters belonging to #mode.
     *
     * Only the member matching #mode is populated; a getter zeroes the rest.
     * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_DDSAMPLING and
     * #RBRINSTRUMENTGEN4_SCHEDULE_MODE_REGIMES have no member: a getter
     * leaves this zeroed and a setter gives #RBRINSTRUMENTGEN4_UNSUPPORTED.
     */
    union
    {
        /**
         * \brief Parameters for
         *        #RBRINSTRUMENTGEN4_SCHEDULE_MODE_CONTINUOUS.
         */
        RBRInstrumentGen4ScheduleModeContinuous continuous;

        /** \brief Parameters for the bursting modes. */
        RBRInstrumentGen4ScheduleModeBursting bursting;
    } parameters;
} RBRInstrumentGen4Schedule;

/**
 * \brief Populate the parameters of a schedule.
 *
 * The caller sets RBRInstrumentGen4Schedule.label to select the schedule.
 * 
 * \p schedule.storage is set to #RBRINSTRUMENTGEN4_UNKNOWN_SCHEDULE_STORAGE for 
 * instruments that do not report the `storage` parameter
 *
 * \note Issues the `schedule <schedule_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] schedule the schedule to read, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the schedule does not exist
 * \see RBRInstrumentGen4_getSchedulePool()
 * \see RBRInstrumentGen4_setSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSchedule(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Schedule *schedule);

/**
 * \brief Set the parameters of a schedule.
 *
 * `storage` is only available on some instrument configurations.
 *
 * \note Issues the `schedule <schedule_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] schedule the schedule to write
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the schedule cannot be
 *                                           written, or when `storage` is set
 *                                           where it is unavailable
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    a count is out of range,
 *                                                    a group label is empty,
 *                                                    or the mode is not a
 *                                                    single known flag
 * \return #RBRINSTRUMENTGEN4_UNSUPPORTED when the mode is `ddsampling` or
 *                                        `regimes`
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the command does not fit
 * \see RBRInstrumentGen4_getSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setSchedule(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Schedule *schedule);

/**
 * \brief `schedule` command parameters.
 *
 * \see RBRInstrumentGen4_getSchedulePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRInstrumentGen4SchedulePool
{
    /**
     * \brief The number of schedules defined on the instrument.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX)` to
     * avoid an out-of-bounds error when accessing #pool if 
     * #maxCount > #RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX.
     */
    int32_t count;

    /**
     * \brief The maximum number of schedules that can exist on the
     * instrument.
     */
    int32_t maxCount;

    /** \brief The pool of schedules. */
    RBRInstrumentGen4Schedule pool[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX];

    /** \brief The modes the instrument offers. */
    RBRInstrumentGen4ScheduleMode availableModes;

    /**
     * \brief The number of entries in #availableFastPeriods.
     *
     * \warning Use `min(availableFastPeriodCount,
     * RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX)` to avoid an
     * out-of-bounds error when accessing #availableFastPeriods if
     * #availableFastPeriodCount >
     * RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX.
     */
    int32_t availableFastPeriodCount;

    /**
     * \brief `availablefastperiods`, in the order reported.
     *
     * Entries past #RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX are
     * discarded.
     */
    RBRInstrumentGen4Period
        availableFastPeriods[RBRINSTRUMENTGEN4_AVAILABLE_FAST_PERIODS_MAX];

    /** \brief `maxregimes`. */
    int32_t maxRegimes;
} RBRInstrumentGen4SchedulePool;

/**
 * \brief Get a human-readable string name for a schedule mode.
 *
 * \param [in] mode the schedule mode
 * \return a string name for the schedule mode
 * \see RBRInstrumentGen4Error_name() for a description of the format of names
 */
const char *RBRInstrumentGen4ScheduleMode_name(
    RBRInstrumentGen4ScheduleMode mode);

/**
 * \brief Populate the pool of the instrument's schedules.
 *
 * Only the labels are reported; read a schedule's parameters with
 * RBRInstrumentGen4_getSchedule().
 *
 * \note Issues the `schedule` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] schedulePool the populated pool of schedules
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedules are successfully read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getSchedulePool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4SchedulePool *schedulePool);

/**
 * \brief Create a schedule with default parameters.
 *
 * Configure it with RBRInstrumentGen4_setSchedule().
 *
 * \note Issues the `schedule create <schedule_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label to give the new schedule
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully created
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when it cannot be created
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createSchedule(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete a schedule.
 *
 * \note Issues the `schedule delete <schedule_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label of the schedule to delete
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedule is successfully deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when it does not exist
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteScheduleAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteSchedule(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete every schedule.
 *
 * \note Issues the `schedule delete all` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the schedules are deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_deleteSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteScheduleAll(
    RBRInstrumentGen4 *instrument);

/**
 * \brief `config <config_label>` command parameters.
 *
 * \see RBRInstrumentGen4ConfigPool
 * \see RBRInstrumentGen4_getConfig()
 * \see RBRInstrumentGen4_setConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRInstrumentGen4Config
{
    /**
     * \brief The configuration's label.
     *
     * Set by the caller to select the configuration to read.
     */
    char label[RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of schedules in the configuration. */
    int32_t scheduleCount;

    /** \brief The labels of the schedules in the configuration. */
    char scheduleList[RBRINSTRUMENTGEN4_SCHEDULE_COUNT_MAX]
                     [RBRINSTRUMENTGEN4_LABEL_NAME_MAX + 1];
} RBRInstrumentGen4Config;

/**
 * \brief Populate the parameters of a configuration.
 *
 * The caller sets RBRInstrumentGen4Config.label to select the configuration.
 *
 * \note Issues the `config <config_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in,out] config the configuration to read, selected by its label
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configuration is read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the configuration does not
 *                                           exist
 * \see RBRInstrumentGen4_getConfigPool()
 * \see RBRInstrumentGen4_setConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfig(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4Config *config);

/**
 * \brief Set the schedules in a configuration.
 *
 * Sends `schedulelist`, the command's only parameter. A zero
 * RBRInstrumentGen4Config.scheduleCount sends `none`.
 *
 * \note Issues the `config <config_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] config the configuration to write
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configuration is written
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when the configuration cannot be
 *                                           written
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    the schedule count is out
 *                                                    of range, or a schedule
 *                                                    label is empty
 * \return #RBRINSTRUMENTGEN4_BUFFER_TOO_SMALL when the list does not fit
 * \see RBRInstrumentGen4_getConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_setConfig(
    RBRInstrumentGen4 *instrument,
    const RBRInstrumentGen4Config *config);

/**
 * \brief `config` command parameters.
 *
 * \see RBRInstrumentGen4_getConfigPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRInstrumentGen4ConfigPool
{
    /**
     * \brief The number of configurations defined on the instrument.
     *
     * \warning Use `min(count, RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX)` to avoid
     * an out-of-bounds error when accessing #pool if 
     * #maxCount > #RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX.
     */
    int32_t count;

    /**
     * \brief The maximum number of configurations that can exist on the
     * instrument.
     */
    int32_t maxCount;

    /** \brief The pool of configurations. */
    RBRInstrumentGen4Config pool[RBRINSTRUMENTGEN4_CONFIG_COUNT_MAX];
} RBRInstrumentGen4ConfigPool;

/**
 * \brief Populate the pool of the instrument's configurations.
 *
 * Only the labels are reported; read a configuration's parameters with
 * RBRInstrumentGen4_getConfig().
 *
 * \note Issues the `config` command.
 *
 * \param [in] instrument the instrument connection
 * \param [out] configPool the populated pool of configurations
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configurations are read
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_getConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_getConfigPool(
    RBRInstrumentGen4 *instrument,
    RBRInstrumentGen4ConfigPool *configPool);

/**
 * \brief Create an empty configuration.
 *
 * Add schedules with RBRInstrumentGen4_setConfig().
 *
 * \note Issues the `config create <config_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label to give the new configuration
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configuration is created
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when it cannot be created
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_createConfig(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete a configuration.
 *
 * \note Issues the `config delete <config_label>` command.
 *
 * \param [in] instrument the instrument connection
 * \param [in] label the label of the configuration to delete
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configuration is deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRINSTRUMENTGEN4_HARDWARE_ERROR when it does not exist
 * \return #RBRINSTRUMENTGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRInstrumentGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfig(
    RBRInstrumentGen4 *instrument,
    const char *label);

/**
 * \brief Delete every configuration.
 *
 * \note Issues the `config delete all` command.
 *
 * \param [in] instrument the instrument connection
 * \return #RBRINSTRUMENTGEN4_SUCCESS when the configurations are deleted
 * \return #RBRINSTRUMENTGEN4_TIMEOUT when a timeout occurs
 * \return #RBRINSTRUMENTGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRInstrumentGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRInstrumentGen4Error RBRInstrumentGen4_deleteConfigAll(
    RBRInstrumentGen4 *instrument);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRINSTRUMENTGEN4CONFIGURATION_H */
