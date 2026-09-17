/**
 * \file RBRGen4Configuration.h
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

#ifndef LIBRBR_RBRGEN4CONFIGURATION_H
#define LIBRBR_RBRGEN4CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/** \brief The maximum number of schedules count. */
#define RBRGEN4_SCHEDULE_COUNT_MAX 16

/** \brief The maximum number of permissionlist count. */
#define RBRGEN4_PERMISSION_COUNT_MAX 14

/** \brief The maximum schedule period in milliseconds. */
#define RBRGEN4_SAMPLING_PERIOD_MAX 86400000

/** \brief The maximum regime boundary in dbar. */
#define RBRGEN4_REGIME_BOUNDARY_MAX 65535

/** \brief The maximum regime bin size in dbar. */
#define RBRGEN4_REGIME_BINSIZE_MAX 6553.5

/** \brief The maximum sampling period within a regime. */
#define RBRGEN4_REGIME_SAMPLING_PERIOD_MAX 65000

/**
 * \brief The maximum number of coefficients in a calibration group.
 *
 * All three groups share one pool of this size on the instrument, so the
 * three counts sum to no more than this.
 *
 * \see RBRGen4Calibration.a
 * \see RBRGen4Calibration.b
 * \see RBRGen4Calibration.m
 */
#define RBRGEN4_CALIBRATION_COEFFICIENT_MAX 14

/**
 * \brief The maximum number of characters in a calibration equation name.
 *
 * Does not include any null terminator.
 * \see RBRGen4Calibration.equation
 */
#define RBRGEN4_CALIBRATION_EQUATION_MAX 32

/** \brief The minimum input timeout. */
#define RBRGEN4_INPUT_TIMEOUT_MIN 10000

/** \brief The maximum input timeout. */
#define RBRGEN4_INPUT_TIMEOUT_MAX 240000

/**
 * \brief The maximum number of configs count.
 * \see RBRGen4ConfigPool.pool
 */
#define RBRGEN4_CONFIG_COUNT_MAX 16

/**
 * \brief The maximum number of groups count.
 * \see RBRGen4GroupPool.pool
 */
#define RBRGEN4_GROUP_COUNT_MAX 16

/** \brief The maximum number of characters in a bus address.
 * The bus address is from 0 to 255, with some reserved addresses.
 */
#define RBRGEN4_BUS_ADDRESS_MAX 3

/** \brief The maximum number of fast periods. */
#define RBRGEN4_AVAILABLE_FAST_PERIODS_MAX 4

/**
 * \brief The maximum number of nodes.
 * \see RBRGen4NodePool.pool
 */
#define RBRGEN4_NODE_COUNT_MAX 12

/**
 * \brief The maximum number of ports.
 * \see RBRGen4Node.portList
 */
#define RBRGEN4_PORT_COUNT_MAX 16

/**
 * \brief The maximum number of devices.
 * \see RBRGen4Port.deviceList
 */
#define RBRGEN4_DEVICE_COUNT_MAX 16

/**
 * \brief `node <node_label>` command parameters.
 *
 * A node is a front-end PCBA, plus the `self` node standing for the main CPU
 * board. Nodes are the top of the instrument's configuration hierarchy: a node
 * carries ports, a port carries devices, and a device exposes channels.
 *
 * \see RBRGen4NodePool
 * \see RBRGen4_getNode()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4Node {
    /**
     * \brief Node label.
     *
     * Set by the caller to select the node to read; see
     * RBRGen4_getNode().
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the PCBA implementing the node. */
    char pcba[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of ports on the node. */
    int32_t portCount;

    /** \brief The labels of the ports on the node. */
    char portList[RBRGEN4_PORT_COUNT_MAX][RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The firmware version running on the node. */
    char fwversion[RBRGEN4_ID_VERSION_MAX + 1];

    /** \brief The node firmware version in semantic-version form. */
    char semver[RBRGEN4_ID_SEMVER_MAX + 1];

    /**
     * \brief The firmware type running on the node.
     *
     * Zero when the node runs no firmware of its own, which it reports as
     * `na`.
     */
    int32_t fwtype;

    /** \brief The time in milliseconds the node takes to power up. */
    int32_t powerUpTime;

    /**
     * \brief The time in milliseconds to wait after powering the node up
     * before powering anything beneath it.
     */
    int32_t inrushOffsetTime;
} RBRGen4Node;

/**
 * \brief Populate the parameters of a node.
 *
 * The caller sets RBRGen4Node.label to select the node to read.
 *
 * \note Issues the `node <node_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] node the node to read
 * \return #RBRGEN4_SUCCESS when the node is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getNodePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getNode(RBRGen4 *conn, RBRGen4Node *node);

/**
 * \brief `node` command parameters.
 *
 * \see RBRGen4_getNodePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4NodePool {
    /**
     * \brief The number of nodes on the instrument.
     *
     * \warning Use `min(count, RBRGEN4_NODE_COUNT_MAX)` to avoid an
     * out-of-bounds error when accessing #pool if
     * #count > #RBRGEN4_NODE_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of nodes. */
    RBRGen4Node pool[RBRGEN4_NODE_COUNT_MAX];
} RBRGen4NodePool;

/**
 * \brief Populate the pool of the instrument's nodes.
 *
 * Only the labels are reported; read the rest of a node's parameters with
 * RBRGen4_getNode().
 *
 * \note Issues the `node` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] nodePool the populated pool of nodes
 * \return #RBRGEN4_SUCCESS when the nodes are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getNode()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getNodePool(RBRGen4 *conn, RBRGen4NodePool *nodePool);

/**
 * \brief The classes of port.
 *
 * \see RBRGen4Port.portClass
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRGen4PortClass {
    /** A direct ADC connection with no bus; typical of on-board sensors. */
    RBRGEN4_PORT_CLASS_VIRTUAL,
    /** A bus-attached port, which speaks one of the port protocols. */
    RBRGEN4_PORT_CLASS_SERIAL,

    /** The number of specific port classes. */
    RBRGEN4_PORT_CLASS_COUNT,
    /** An unknown or unrecognized port class. */
    RBRGEN4_UNKNOWN_PORT_CLASS
} RBRGen4PortClass;

/**
 * \brief Get a human-readable string name for a port class.
 *
 * \param [in] portClass the port class
 * \return a string name for the port class
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4PortClass_name(RBRGen4PortClass portClass);

/**
 * \brief The protocols a port can speak.
 *
 * A `virtual` port speaks none of these. Consult
 * RBRGen4Port.availableProtocols for the protocols a given port is
 * capable of.
 *
 * \see RBRGen4Port
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRGen4PortProtocol {
    /** An unrecognized protocol, or none being spoken. */
    RBRGEN4_PORT_PROTOCOL_NONE = 0,
    /** Pressure. */
    RBRGEN4_PORT_PROTOCOL_PRESSURE = 1 << 0,
    /** RBR serial. */
    RBRGEN4_PORT_PROTOCOL_RBRSERIAL = 1 << 1,
    /** RBR modem. */
    RBRGEN4_PORT_PROTOCOL_RBRMODEM = 1 << 2,
    /** RBR multidrop. */
    RBRGEN4_PORT_PROTOCOL_RBRMULTIDROP = 1 << 3,
    /** Corresponds to the largest port protocol enum value. */
    RBRGEN4_PORT_PROTOCOL_MAX = RBRGEN4_PORT_PROTOCOL_RBRMULTIDROP
} RBRGen4PortProtocol;

/**
 * \brief Get a human-readable string name for a port protocol.
 *
 * \param [in] protocol the port protocol
 * \return a string name for the port protocol
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4PortProtocol_name(RBRGen4PortProtocol protocol);

/**
 * \brief `port <port_label>` command parameters.
 *
 * A port is an attachment point on a node to which devices attach.
 *
 * \see RBRGen4PortPool
 * \see RBRGen4_getPort()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4Port {
    /**
     * \brief Port label.
     *
     * Set by the caller to select the port to read; see
     * RBRGen4_getPort().
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the node the port belongs to. */
    char node[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The class of the port. */
    RBRGen4PortClass portClass;

    /** \brief The protocol currently selected on the port. */
    RBRGen4PortProtocol protocol;

    /**
     * \brief Protocols the port is capable of speaking.
     *
     * Treated as a bit field representation of available protocols as defined
     * by RBRGen4PortProtocol. For details, consult
     * the Working with Bit Fields page of the documentation.
     */
    RBRGen4PortProtocol availableProtocols;

    /**
     * \brief The baud rate of the port.
     *
     * Zero for a `virtual` port.
     */
    int32_t baudRate;

    /** \brief The number of devices attached to the port. */
    int32_t deviceCount;

    /** \brief The labels of the devices attached to the port. */
    char deviceList[RBRGEN4_DEVICE_COUNT_MAX][RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The time in milliseconds to bring power to the port. */
    int32_t powerUpTime;
} RBRGen4Port;

/**
 * \brief Populate the parameters of a port.
 *
 * The caller sets RBRGen4Port.label to select the port to read.
 *
 * \note Issues the `port <port_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] port the port to read
 * \return #RBRGEN4_SUCCESS when the port is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getPortPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getPort(RBRGen4 *conn, RBRGen4Port *port);

/**
 * \brief `port` command parameters.
 *
 * \see RBRGen4_getPortPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4PortPool {
    /**
     * \brief The number of ports across all nodes.
     *
     * \warning Use `min(count, RBRGEN4_PORT_COUNT_MAX)` to avoid an
     * out-of-bounds error when accessing #pool if
     * #count > #RBRGEN4_PORT_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of ports. */
    RBRGen4Port pool[RBRGEN4_PORT_COUNT_MAX];
} RBRGen4PortPool;

/**
 * \brief Populate the pool of the instrument's ports.
 *
 * Only the labels are reported; read the rest of a port's parameters with
 * RBRGen4_getPort().
 *
 * \note Issues the `port` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] portPool the populated pool of ports
 * \return #RBRGEN4_SUCCESS when the ports are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getPort()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getPortPool(RBRGen4 *conn, RBRGen4PortPool *portPool);

/**
 * \brief The classes of device.
 *
 * \see RBRGen4Device.deviceClass
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef enum RBRGen4DeviceClass {
    /** A device which measures. */
    RBRGEN4_DEVICE_CLASS_SENSOR,
    /** A device which acts, such as a valve or a UV LED. */
    RBRGEN4_DEVICE_CLASS_ACTUATOR,
    /** A device which raises asynchronous events. */
    RBRGEN4_DEVICE_CLASS_EVENTGEN,
    /** A device which carries other devices on a multidrop bus. */
    RBRGEN4_DEVICE_CLASS_MODEM,

    /** The number of specific device classes. */
    RBRGEN4_DEVICE_CLASS_COUNT,
    /** An unknown or unrecognized device class. */
    RBRGEN4_UNKNOWN_DEVICE_CLASS
} RBRGen4DeviceClass;

/**
 * \brief Get a human-readable string name for a device class.
 *
 * \param [in] deviceClass the device class
 * \return a string name for the device class
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4DeviceClass_name(RBRGen4DeviceClass deviceClass);

/**
 * \brief `device <device_label>` command parameters.
 *
 * A device is a logical sensor, actuator, event source, or modem attached to a
 * port. Devices are produced by discovery: there is no command to create one.
 *
 * \see RBRGen4DevicePool
 * \see RBRGen4_getDevice()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4Device {
    /**
     * \brief Device label.
     *
     * Set by the caller to select the device to read; see
     * RBRGen4_getDevice().
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The label of the port the device is attached to. */
    char port[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The class of the device. */
    RBRGen4DeviceClass deviceClass;

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
    char pn[RBRGEN4_PART_NUMBER_MAX + 1];

    /** \brief The firmware version running on the device. */
    char fwversion[RBRGEN4_ID_VERSION_MAX + 1];

    /**
     * \brief The firmware type running on the device.
     *
     * Zero when the device runs no firmware of its own, which it reports as
     * `na`.
     */
    int32_t fwtype;

    /** \brief The generic name of the kind of device installed. */
    char name[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief The number of channels the device exposes. */
    int32_t channelCount;

    /**
     * \brief The labels of the channels the device exposes.
     *
     * A device can name a channel which the `channel` command does not
     * enumerate and will not accept, so a label found here is not
     * necessarily readable with RBRGen4_getChannel().
     */
    char channelList[RBRGEN4_CHANNEL_MAX][RBRGEN4_LABEL_NAME_MAX + 1];

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
} RBRGen4Device;

/**
 * \brief Populate the parameters of a device.
 *
 * The caller sets RBRGen4Device.label to select the device to read.
 *
 * \note Issues the `device <device_label>` command with every
 *       parameter named explicitly.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] device the device to read
 * \return #RBRGEN4_SUCCESS when the device is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getDevice(RBRGen4 *conn, RBRGen4Device *device);

/**
 * \brief `device` command parameters.
 *
 * \see RBRGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
typedef struct RBRGen4DevicePool {
    /**
     * \brief The number of devices across all ports.
     *
     * \warning Use `min(count, RBRGEN4_DEVICE_COUNT_MAX)` to avoid
     * an out-of-bounds error when accessing #pool if
     * #count > #RBRGEN4_DEVICE_COUNT_MAX.
     */
    int32_t count;

    /** \brief The pool of devices. */
    RBRGen4Device pool[RBRGEN4_DEVICE_COUNT_MAX];
} RBRGen4DevicePool;

/**
 * \brief Populate the pool of the instrument's devices.
 *
 * Only the labels are reported; read the rest of a device's parameters with
 * RBRGen4_getDevice().
 *
 * \note Issues the `device` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] devicePool the populated pool of devices
 * \return #RBRGEN4_SUCCESS when the devices are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_getDevice()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_getDevicePool(RBRGen4 *conn, RBRGen4DevicePool *devicePool);

/**
 * \brief Sweep every port and repopulate the devices attached to them.
 *
 * Discovery reports every device present after the sweep, not only the ones it
 * has just added, so it fills the same pool RBRGen4_getDevicePool()
 * does. Both report nothing but the labels; read a device's parameters with
 * RBRGen4_getDevice().
 *
 * \note Issues the `device discover` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] devicePool the labels of the devices present after the sweep
 * \return #RBRGEN4_SUCCESS when discovery completes
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the instrument refuses, or another
 *                                      hardware error occurs
 * \see RBRGen4_getDevice()
 * \see RBRGen4_getDevicePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830109/Configuration+information+and+calibration
 */
RBRGen4Error RBRGen4_discoverDevices(RBRGen4 *conn, RBRGen4DevicePool *devicePool);

/**
 * \brief `calibration <channel_label>` command parameters.
 *
 * The number and kind of coefficients depend on the channel's equation.
 * Coefficients the equation does not use are absent from the response and the
 * instrument rejects them, so a group's count bounds what is present.
 *
 * \see RBRGen4Channel
 * \see RBRGen4_getCalibration()
 * \see RBRGen4_setCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
typedef struct RBRGen4Calibration {
    /**
     * \brief The label of the channel the calibration belongs to.
     *
     * Set by the caller to select the calibration to read; see
     * RBRGen4_getCalibration(). Calibrations are one to one with
     * channels and cannot be created or deleted.
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The formula used to convert raw readings to physical units.
     *
     * E.g. `temperature`, `linear`, `deri_depth`.
     *
     * \readonly
     */
    char equation[RBRGEN4_CALIBRATION_EQUATION_MAX + 1];

    /**
     * \brief The date and time of the calibration.
     *
     * `20000101000000` means the channel has never been calibrated. The
     * instrument restamps this with the current time when coefficients change
     * and no date is sent with them.
     */
    RBRGen4DateTime dateTime;

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
    float a[RBRGEN4_CALIBRATION_COEFFICIENT_MAX];

    /** \brief The number of b coefficients the equation uses. */
    int32_t bCount;

    /**
     * \brief The b coefficients.
     *
     * \warning Intended to be changed by RBR or an expert user only.
     */
    float b[RBRGEN4_CALIBRATION_COEFFICIENT_MAX];

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
     * \see RBRGen4_getParameters()
     */
    char m[RBRGEN4_CALIBRATION_COEFFICIENT_MAX][RBRGEN4_LABEL_NAME_MAX + 1];
} RBRGen4Calibration;

/** \brief An internal module identifier. */
typedef uint8_t RBRGen4ModuleAddress;

/**
 * \brief Whether a channel carries a measurement or an instrument housekeeping
 * value.
 *
 * \see RBRGen4Channel
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef enum RBRGen4ChannelNature {
    /** The channel measures a physical parameter. */
    RBRGEN4_CHANNEL_NATURE_SCIENTIFIC,
    /** The channel reports an instrument housekeeping value. */
    RBRGEN4_CHANNEL_NATURE_SYSTEM,
    /** The number of specific channel natures. */
    RBRGEN4_CHANNEL_NATURE_COUNT,
    /** An unknown or unrecognized channel nature. */
    RBRGEN4_UNKNOWN_CHANNEL_NATURE
} RBRGen4ChannelNature;

/**
 * \brief Get a human-readable string name for a channel nature.
 *
 * \param [in] nature the channel nature
 * \return a string name for the channel nature
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4ChannelNature_name(RBRGen4ChannelNature nature);

/**
 * \brief Instrument `channel <channel_label>` command parameters.
 *
 * \see RBRGen4_getChannel()
 * \see RBRGen4_setChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef struct RBRGen4Channel {
    /**
     * \brief The channel's label.
     *
     * Set by the caller to select the channel to read; see
     * RBRGen4_getChannel().
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief A short, pre-defined generic name for the installed channel.
     *
     * E.g. `temp006`, `pres003`, `dpth001`.
     *
     * \readonly
     */
    char type[RBRGEN4_CHANNEL_TYPE_MAX + 1];

    /**
     * \brief Settling time in milliseconds; zero on a derived channel.
     *
     * \readonly
     */
    RBRGen4Period settlingTime;

    /**
     * \brief Measuring time in milliseconds; zero on a derived channel.
     *
     * \readonly
     */
    RBRGen4Period measuringTime;

    /**
     * \brief Read-out time in milliseconds; zero on a derived channel.
     *
     * \readonly
     */
    RBRGen4Period readOutTime;

    /**
     * \brief The unit in which processed data is reported.
     *
     * The only parameter of the command a caller may change.
     */
    char userUnits[RBRGEN4_CHANNEL_UNIT_MAX + 1];

    /**
     * \brief Whether the channel measures or reports housekeeping.
     *
     * \readonly
     */
    RBRGen4ChannelNature nature;

    /**
     * \brief Whether the channel is computed from other channels rather than
     * measured.
     *
     * \readonly
     */
    bool derived;

    /**
     * \brief The label of the node the channel is reached through.
     *
     * `self` for a channel of the instrument itself, and empty for a derived
     * channel, which the instrument reports as `na`.
     *
     * \readonly
     */
    char node[RBRGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The label of the port the channel is reached through.
     *
     * Empty for a derived channel, which the instrument reports as `na`.
     *
     * \readonly
     */
    char port[RBRGEN4_LABEL_NAME_MAX + 1];

    /**
     * \brief The label of the device the channel belongs to.
     *
     * Empty for a derived channel, which the instrument reports as `na`.
     *
     * \readonly
     */
    char device[RBRGEN4_LABEL_NAME_MAX + 1];
} RBRGen4Channel;

/**
 * \brief `channel` command parameters. The `list` is stored in a user provided
 * buffer (#pool).
 *
 * \see RBRGen4_getChannelPool()
 * \see RBRGen4_getChannelPoolByNature()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
typedef struct RBRGen4ChannelPool {
    /** \brief The number of channels #pool can hold. */
    int32_t size;

    /**
     * \brief The number of channels reported.
     *
     * \warning This field will be larger than #size when
     * #RBRGEN4_TRUNCATED is returned by the getter. Care should be
     * taken to avoid out-of-bounds access when iterating over #pool.
     */
    int32_t count;

    /**
     * \brief User provided buffer of the channels reported.
     *
     * Discovery reports nothing but the labels; read a channel's parameters
     * with RBRGen4_getChannel().
     */
    RBRGen4Channel *pool;
} RBRGen4ChannelPool;

/**
 * \brief Populate the parameters of a channel.
 *
 * The caller sets RBRGen4Channel.label to select the channel to
 * read.
 *
 * \note Issues the `channel <channel_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] channel the channel to read, selected by its label
 * \return #RBRGEN4_SUCCESS when the channel is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_HARDWARE_ERROR when the channel does not exist, or another
 *                                      hardware error occurs
 * \see RBRGen4_getChannelPool()
 * \see RBRGen4_setChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRGen4Error RBRGen4_getChannel(RBRGen4 *conn, RBRGen4Channel *channel);

/**
 * \brief Update a channel's user units.
 *
 * RBRGen4Channel.userUnits is the only parameter of the command a
 * caller may change; every other field of the structure is read-only. Read
 * the channel with RBRGen4_getChannel(), change the units, and write
 * the structure back.
 *
 * \note Issues the `channel <channel_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] channel the channel to write, selected by its label
 * \return #RBRGEN4_SUCCESS when the channel is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the channel cannot be changed, or
 *                                      another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the units are empty
 * \see RBRGen4_getChannel()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRGen4Error RBRGen4_setChannel(RBRGen4 *conn, const RBRGen4Channel *channel);

/**
 * \brief Read the labels of the channels configured on the instrument.
 *
 * Reports nothing but the labels; read a channel's parameters with
 * RBRGen4_getChannel().
 *
 * \note Issues the `channel` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] channelPool the channels present, labels only
 * \return #RBRGEN4_SUCCESS when the pool is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a channelPool cannot hold every
 *                                      reported channel; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the channel pool cannot be read, or
 *                                      another hardware error occurs
 * \see RBRGen4_getChannelPoolByNature()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRGen4Error RBRGen4_getChannelPool(RBRGen4 *conn, RBRGen4ChannelPool *channelPool);

/**
 * \brief Read the labels of the channels of one nature.
 *
 * Reports nothing but the labels; read a channel's parameters with
 * RBRGen4_getChannel().
 *
 * \note Issues the `channel scientific` or `channel system` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] nature the nature of the channels to report
 * \param [in,out] channelPool the channels present, labels only
 * \return #RBRGEN4_SUCCESS when the pool is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a channelPool cannot hold every
 *                                      reported channel; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the channel pool cannot be read, or
 *                                      another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the nature is not
 *                                                    one the command accepts
 * \see RBRGen4_getChannelPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/47153202/channel
 */
RBRGen4Error RBRGen4_getChannelPoolByNature(RBRGen4 *conn, RBRGen4ChannelNature nature,
                                            RBRGen4ChannelPool *channelPool);

/**
 * \brief Read a channel's calibration.
 *
 * The caller sets RBRGen4Calibration.label to select the channel.
 *
 * \note Issues the `calibration <channel_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] calibration the calibration to read, selected by its label
 * \return #RBRGEN4_SUCCESS when the calibration is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the channel does not exist, or another
 *                                      hardware error occurs
 * \see RBRGen4_setCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
RBRGen4Error RBRGen4_getCalibration(RBRGen4 *conn, RBRGen4Calibration *calibration);

/**
 * \brief Update a channel's calibration.
 *
 * Sends the date, the offset and slope, and every a, b, and m coefficient the
 * equation uses. Read the calibration with RBRGen4_getCalibration(),
 * change what you need, and write the structure back: the counts read there
 * are what bounds the coefficients sent.
 *
 * The equation is read-only and never sent; the instrument rejects a write to
 * it.
 *
 * \warning Hardware errors may occur if the instrument is logging, a
 *          coefficient is out of range for the equation, or an m reference
 *          does not name something the equation can use.
 *
 * \param [in] conn the instrument connection
 * \param [in] calibration the calibration to write, selected by its label
 * \return #RBRGEN4_SUCCESS when the calibration is successfully
 *                                    written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the calibration cannot be changed, or
 *                                      another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when a coefficient count
 *                                                    is out of range
 * \see RBRGen4_getCalibration()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828510/calibration
 */
RBRGen4Error RBRGen4_setCalibration(RBRGen4 *conn, const RBRGen4Calibration *calibration);

/**
 * \brief Instrument `settings` command parameters.
 *
 * \see RBRGen4_getSettings()
 * \see RBRGen4_setSettings()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 */
typedef struct RBRGen4Settings {
    /**
     * \brief Whether the instrument returns the “Ready:” prompt following a
     * response.
     */
    bool prompt;

    /**
     * \brief Whether the instrument returns a response from a create or
     * set/modify operation to verify the new state.
     */
    bool confirmation;

    /**
     * \brief The delay in milliseconds between the completion of a poll and
     * the removal of sensor power.
     */
    RBRGen4Period pollPowerOffDelay;
} RBRGen4Settings;

/**
 * \brief Get miscellaneous logger settings
 * \note Issues the `settings` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] settings the logger settings
 * \return #RBRGEN4_SUCCESS when the setting is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setSettings()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 */
RBRGen4Error RBRGen4_getSettings(RBRGen4 *conn, RBRGen4Settings *settings);

/**
 * \brief Set the miscellaneous logger settings.
 * \note Issues the `settings` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] settings the values for the settings in the logger
 * \return #RBRGEN4_SUCCESS when the setting is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the settings cannot be changed, or
 *                                      another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the power-off delay
 *                                                    is negative
 * \warning The library expects both \a prompt and \a confirmation to be on.
 *          With \a confirmation off the instrument answers a set with nothing
 *          at all, and every later setter blocks until the command timeout.
 * \see RBRGen4_getSettings()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13828508/settings
 */
RBRGen4Error RBRGen4_setSettings(RBRGen4 *conn, const RBRGen4Settings *settings);

/**
 * \brief `parameters` command parameters.
 *
 * \see RBRGen4_getParameters()
 * \see RBRGen4_setParameters()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 */
typedef struct RBRGen4Parameters {
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
} RBRGen4Parameters;

/**
 * \brief Get parameters which may be required when computing calibrated output.
 * \note Issues the `parameters` command.
 *
 * \param [in] conn the instrument connection
 * \param [out] parameters the parameters in the logger
 * \return #RBRGEN4_SUCCESS when the parameters are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_setParameters()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 */
RBRGen4Error RBRGen4_getParameters(RBRGen4 *conn, RBRGen4Parameters *parameters);

/**
 * \brief Set parameters which may be required when computing calibrated output.
 * \note Issues the `parameters` command.
 *
 * \warning Hardware errors may occur if the instrument is logging.
 *
 * \param [in] conn the instrument connection
 * \param [in] parameters the values for the parameters in the logger
 * \return #RBRGEN4_SUCCESS when the parameters are successfully
 *                                    written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the parameters cannot be changed, or
 *                                      another hardware error occurs
 * \see RBRGen4_getParameters()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/42729486/parameters
 */
RBRGen4Error RBRGen4_setParameters(RBRGen4 *conn, const RBRGen4Parameters *parameters);

/**
 * \brief `group <group_label>` command parameters.
 *
 * \see RBRGen4GroupPool
 * \see RBRGen4_getGroup()
 * \see RBRGen4_setGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
typedef struct RBRGen4Group {
    /**
     * \brief The group's label.
     *
     * Set by the caller to select the group to read.
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];
} RBRGen4Group;

/**
 * \brief Read the channels in a group.
 *
 * The caller sets RBRGen4Group.label to select the group to read.
 * The labels of the group's channels are written to \a channelList when it
 * is given.
 *
 * \note Issues the `group <group_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] group the group to read, selected by its label
 * \param [out] channelList the channels in the group, or `NULL` to skip them
 * \return #RBRGEN4_SUCCESS when the group is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_TRUNCATED when \a channelList cannot hold every
 *                                      reported channel; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the group does not exist, or another
 *                                      hardware error occurs
 * \see RBRGen4_getGroupPool()
 * \see RBRGen4_setGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_getGroup(RBRGen4 *conn, RBRGen4Group *group, RBRGen4LabelList *channelList);

/**
 * \brief Set the channels in a group.
 *
 * Sends `channellist`, the only writable parameter. An empty \a channelList
 * sends `none`.
 *
 * \note Issues the `group <group_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] group the group to write, selected by its label
 * \param [in] channelList the channels to put in the group
 * \return #RBRGEN4_SUCCESS when the group is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the group cannot be written, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    \a channelList is `NULL`,
 *                                                    its count does not fit
 *                                                    its array, or a channel
 *                                                    label is empty
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the list does not fit the
 *                                            command
 * \see RBRGen4_getGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_setGroup(RBRGen4 *conn, const RBRGen4Group *group,
                              const RBRGen4LabelList *channelList);

/**
 * \brief `group` command parameters. The `list` is stored in a user provided
 * buffer (#pool).
 *
 * \see RBRGen4_getGroupPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
typedef struct RBRGen4GroupPool {
    /** \brief The number of groups #pool can hold. */
    int32_t size;

    /**
     * \brief The number of groups defined on the instrument.
     *
     * \warning This field will be larger than #size when
     * #RBRGEN4_TRUNCATED is returned by the getter. Care should be
     * taken to avoid out-of-bounds access when iterating over #pool.
     */
    int32_t count;

    /** \brief The maximum number of groups that can exist on the instrument. */
    int32_t maxCount;

    /** \brief User provided buffer of the groups defined. */
    RBRGen4Group *pool;
} RBRGen4GroupPool;

/**
 * \brief Populate the pool of the instrument's groups.
 *
 * Only the labels are reported; read a group's parameters with
 * RBRGen4_getGroup().
 *
 * \note Issues the `group` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] groupPool the groups defined, labels only
 * \return #RBRGEN4_SUCCESS when the groups are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a groupPool cannot hold every
 *                                      reported group; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \see RBRGen4_getGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_getGroupPool(RBRGen4 *conn, RBRGen4GroupPool *groupPool);

/**
 * \brief Create an empty group.
 *
 * Add channels with RBRGen4_setGroup().
 *
 * \note Issues the `group create <group_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label to give the new group
 * \return #RBRGEN4_SUCCESS when the group is successfully created
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the group cannot be created, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_createGroup(RBRGen4 *conn, const char *label);

/**
 * \brief Delete a group.
 *
 * \note Issues the `group delete <group_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label of the group to delete
 * \return #RBRGEN4_SUCCESS when the group is successfully deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the group does not exist, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteGroupAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_deleteGroup(RBRGen4 *conn, const char *label);

/**
 * \brief Delete every group.
 *
 * \note Issues the `group delete all` command.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the groups are successfully deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_deleteGroup()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/49021024/group
 */
RBRGen4Error RBRGen4_deleteGroupAll(RBRGen4 *conn);

/**
 * \brief The modes of a schedule.
 *
 * Flags, so one type serves both a schedule's mode and the set of modes the
 * instrument offers. A schedule's mode must be a single flag;
 * RBRGen4_setSchedule() rejects any other value.
 *
 * \see RBRGen4Schedule.mode
 * \see RBRGen4SchedulePool.availableModes
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRGen4ScheduleMode {
    /** \brief No mode, and any mode the library does not recognize. */
    RBRGEN4_SCHEDULE_MODE_NONE = 0,
    /** \brief Continuous mode. */
    RBRGEN4_SCHEDULE_MODE_CONTINUOUS = 1 << 0,
    /** \brief Average mode. */
    RBRGEN4_SCHEDULE_MODE_AVERAGE = 1 << 1,
    /** \brief Burst mode. */
    RBRGEN4_SCHEDULE_MODE_BURST = 1 << 2,
    /** \brief Tide mode. */
    RBRGEN4_SCHEDULE_MODE_TIDE = 1 << 3,
    /** \brief Wave mode. */
    RBRGEN4_SCHEDULE_MODE_WAVE = 1 << 4,
    /** \brief Direction-dependent mode. */
    RBRGEN4_SCHEDULE_MODE_DDSAMPLING = 1 << 5,
    /** \brief Regimes mode. */
    RBRGEN4_SCHEDULE_MODE_REGIMES = 1 << 6,
    /** \brief The greatest mode flag. */
    RBRGEN4_SCHEDULE_MODE_MAX = RBRGEN4_SCHEDULE_MODE_REGIMES
} RBRGen4ScheduleMode;

/**
 * \brief Whether a schedule's data is stored in memory.
 *
 * \see RBRGen4Schedule.storage
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRGen4ScheduleStorage {
    /** Data for this schedule is not stored in memory. */
    RBRGEN4_SCHEDULE_STORAGE_OFF,
    /** Data for this schedule is stored in memory. */
    RBRGEN4_SCHEDULE_STORAGE_ON,
    /** The number of specific storage states. */
    RBRGEN4_SCHEDULE_STORAGE_COUNT,
    /** The parameter was not reported. */
    RBRGEN4_UNKNOWN_SCHEDULE_STORAGE
} RBRGen4ScheduleStorage;

/**
 * \brief Get a human-readable string name for a storage state.
 *
 * \param [in] storage the storage state
 * \return a string name for the storage state
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4ScheduleStorage_name(RBRGen4ScheduleStorage storage);

/**
 * \brief A schedule's parameters in
 *        #RBRGEN4_SCHEDULE_MODE_CONTINUOUS.
 *
 * \see RBRGen4Schedule.parameters
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRGen4ScheduleModeContinuous {
    /** \brief `period`, in milliseconds. */
    RBRGen4Period period;
} RBRGen4ScheduleModeContinuous;

/**
 * \brief A schedule's parameters in the bursting modes.
 *
 * #RBRGEN4_SCHEDULE_MODE_AVERAGE,
 * #RBRGEN4_SCHEDULE_MODE_BURST,
 * #RBRGEN4_SCHEDULE_MODE_TIDE and
 * #RBRGEN4_SCHEDULE_MODE_WAVE take the same parameters and so share
 * one structure.
 *
 * \see RBRGen4Schedule.parameters
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRGen4ScheduleModeBursting {
    /** \brief `period`, in milliseconds. */
    RBRGen4Period period;

    /** \brief `measurementperiod`, in milliseconds. */
    RBRGen4Period measurementPeriod;

    /** \brief `measurementcount`. */
    int32_t measurementCount;
} RBRGen4ScheduleModeBursting;

/**
 * \brief Destinations for a schedule's real-time data.
 *
 * \see RBRGen4Schedule.stream
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef enum RBRGen4ScheduleStream {
    /** Data for this schedule is not streamed in real time. */
    RBRGEN4_SCHEDULE_STREAM_OFF,
    /** Data for this schedule is streamed over the USB CDC link. */
    RBRGEN4_SCHEDULE_STREAM_USB,
    /** Data for this schedule is streamed over the serial link. */
    RBRGEN4_SCHEDULE_STREAM_SERIAL,
    /** The number of specific stream destinations. */
    RBRGEN4_SCHEDULE_STREAM_COUNT,
    /** An unknown or unrecognized stream destination. */
    RBRGEN4_UNKNOWN_SCHEDULE_STREAM
} RBRGen4ScheduleStream;

/**
 * \brief Get a human-readable string name for a stream destination.
 *
 * \param [in] stream the stream destination
 * \return a string name for the stream destination
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4ScheduleStream_name(RBRGen4ScheduleStream stream);

/**
 * \brief `schedule <schedule_label>` command parameters.
 *
 * \see RBRGen4SchedulePool
 * \see RBRGen4_getSchedule()
 * \see RBRGen4_setSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRGen4Schedule {
    /**
     * \brief The schedule's label.
     *
     * Set by the caller to select the schedule to read.
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];

    /** \brief Where the schedule's data is streamed in real time. */
    RBRGen4ScheduleStream stream;

    /**
     * \brief Whether the schedule's data is stored in memory.
     *
     * Some instrument configurations do not support data storage, in which case
     * this field should be left as #RBRGEN4_UNKNOWN_SCHEDULE_STORAGE.
     *
     * \see RBRGen4ScheduleStorage
     */
    RBRGen4ScheduleStorage storage;

    /** \brief `castdetection`, which applies in every mode. */
    bool castDetection;

    /**
     * \brief The mode the schedule runs in.
     *
     * Exactly one mode flag.
     */
    RBRGen4ScheduleMode mode;

    /**
     * \brief The parameters belonging to #mode.
     *
     * Only the member matching #mode is populated; a getter zeroes the rest.
     * #RBRGEN4_SCHEDULE_MODE_DDSAMPLING and
     * #RBRGEN4_SCHEDULE_MODE_REGIMES have no member: a getter
     * leaves this zeroed and a setter gives #RBRGEN4_UNSUPPORTED.
     */
    union {
        /**
         * \brief Parameters for
         *        #RBRGEN4_SCHEDULE_MODE_CONTINUOUS.
         */
        RBRGen4ScheduleModeContinuous continuous;

        /** \brief Parameters for the bursting modes. */
        RBRGen4ScheduleModeBursting bursting;
    } parameters;
} RBRGen4Schedule;

/**
 * \brief Populate the parameters of a schedule.
 *
 * The caller sets RBRGen4Schedule.label to select the schedule.
 * The labels of the groups the schedule samples are written to \a groupList
 * when it is given.
 *
 * \p schedule.storage is set to #RBRGEN4_UNKNOWN_SCHEDULE_STORAGE for
 * instruments that do not report the `storage` parameter
 *
 * \note Issues the `schedule <schedule_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] schedule the schedule to read, selected by its label
 * \param [out] groupList the groups the schedule samples, or `NULL` to skip
 *                        them
 * \return #RBRGEN4_SUCCESS when the schedule is successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_TRUNCATED when \a groupList cannot hold every
 *                                      reported group; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the schedule does not exist, or another
 *                                      hardware error occurs
 * \see RBRGen4_getSchedulePool()
 * \see RBRGen4_setSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_getSchedule(RBRGen4 *conn, RBRGen4Schedule *schedule,
                                 RBRGen4LabelList *groupList);

/**
 * \brief Set the parameters of a schedule.
 *
 * `storage` is only available on some instrument configurations. An empty
 * \a groupList sends `none`; a `NULL` \a groupList leaves the instrument's
 * group list unchanged.
 *
 * \note Issues the `schedule <schedule_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] schedule the schedule to write
 * \param [in] groupList the groups the schedule samples, or `NULL` to leave
 *                       them as they are
 * \return #RBRGEN4_SUCCESS when the schedule is successfully written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the schedule cannot be written, or when
 *                                      `storage` is set where it is
 *                                      unavailable, or another hardware error
 *                                      occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    the list's count does
 *                                                    not fit its array, a
 *                                                    group label is empty,
 *                                                    or the mode is not a
 *                                                    single known flag
 * \return #RBRGEN4_UNSUPPORTED when the mode is `ddsampling` or
 *                                        `regimes`
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the command does not fit
 * \see RBRGen4_getSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_setSchedule(RBRGen4 *conn, const RBRGen4Schedule *schedule,
                                 const RBRGen4LabelList *groupList);

/**
 * \brief `schedule` command parameters. The `list` is stored in a user
 * provided buffer (#pool).
 *
 * \see RBRGen4_getSchedulePool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
typedef struct RBRGen4SchedulePool {
    /** \brief The number of schedules #pool can hold. */
    int32_t size;

    /**
     * \brief The number of schedules defined on the instrument.
     *
     * \warning This field will be larger than #size when
     * #RBRGEN4_TRUNCATED is returned by the getter. Care should be
     * taken to avoid out-of-bounds access when iterating over #pool.
     */
    int32_t count;

    /**
     * \brief The maximum number of schedules that can exist on the
     * instrument.
     */
    int32_t maxCount;

    /** \brief User provided buffer of the schedules defined. */
    RBRGen4Schedule *pool;

    /** \brief The modes the instrument offers. */
    RBRGen4ScheduleMode availableModes;

    /**
     * \brief The number of entries in #availableFastPeriods.
     *
     * \warning Use `min(availableFastPeriodCount,
     * RBRGEN4_AVAILABLE_FAST_PERIODS_MAX)` to avoid an
     * out-of-bounds error when accessing #availableFastPeriods if
     * #availableFastPeriodCount >
     * RBRGEN4_AVAILABLE_FAST_PERIODS_MAX.
     */
    int32_t availableFastPeriodCount;

    /**
     * \brief `availablefastperiods`, in the order reported.
     *
     * Entries past #RBRGEN4_AVAILABLE_FAST_PERIODS_MAX are
     * discarded.
     */
    RBRGen4Period availableFastPeriods[RBRGEN4_AVAILABLE_FAST_PERIODS_MAX];

    /** \brief `maxregimes`. */
    int32_t maxRegimes;
} RBRGen4SchedulePool;

/**
 * \brief Get a human-readable string name for a schedule mode.
 *
 * \param [in] mode the schedule mode
 * \return a string name for the schedule mode
 * \see RBRGen4Error_name() for a description of the format of names
 */
const char *RBRGen4ScheduleMode_name(RBRGen4ScheduleMode mode);

/**
 * \brief Populate the pool of the instrument's schedules.
 *
 * Only the labels are reported; read a schedule's parameters with
 * RBRGen4_getSchedule().
 *
 * \note Issues the `schedule` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] schedulePool the schedules defined, labels only
 * \return #RBRGEN4_SUCCESS when the schedules are successfully read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a schedulePool cannot hold every
 *                                      reported schedule; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \see RBRGen4_getSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_getSchedulePool(RBRGen4 *conn, RBRGen4SchedulePool *schedulePool);

/**
 * \brief Create a schedule with default parameters.
 *
 * Configure it with RBRGen4_setSchedule().
 *
 * \note Issues the `schedule create <schedule_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label to give the new schedule
 * \return #RBRGEN4_SUCCESS when the schedule is successfully created
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when it cannot be created, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_createSchedule(RBRGen4 *conn, const char *label);

/**
 * \brief Delete a schedule.
 *
 * \note Issues the `schedule delete <schedule_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label of the schedule to delete
 * \return #RBRGEN4_SUCCESS when the schedule is successfully deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when it does not exist, or another hardware
 *                                      error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteScheduleAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_deleteSchedule(RBRGen4 *conn, const char *label);

/**
 * \brief Delete every schedule.
 *
 * \note Issues the `schedule delete all` command.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the schedules are deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_deleteSchedule()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48890051/schedule
 */
RBRGen4Error RBRGen4_deleteScheduleAll(RBRGen4 *conn);

/**
 * \brief `config <config_label>` command parameters.
 *
 * \see RBRGen4ConfigPool
 * \see RBRGen4_getConfig()
 * \see RBRGen4_setConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRGen4Config {
    /**
     * \brief The configuration's label.
     *
     * Set by the caller to select the configuration to read.
     */
    char label[RBRGEN4_LABEL_NAME_MAX + 1];
} RBRGen4Config;

/**
 * \brief Read the schedules in a configuration.
 *
 * The caller sets RBRGen4Config.label to select the configuration.
 * The labels of the configuration's schedules are written to
 * \a scheduleList when it is given.
 *
 * \note Issues the `config <config_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] config the configuration to read, selected by its label
 * \param [out] scheduleList the schedules in the configuration, or `NULL` to
 *                           skip them
 * \return #RBRGEN4_SUCCESS when the configuration is read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \return #RBRGEN4_TRUNCATED when \a scheduleList cannot hold every
 *                                      reported schedule; the first `size` are
 *                                      stored, and `count` is set to the value
 *                                      reported by the instrument which WILL
 *                                      exceed `size`
 * \return #RBRGEN4_HARDWARE_ERROR when the configuration does not exist, or
 *                                      another hardware error occurs
 * \see RBRGen4_getConfigPool()
 * \see RBRGen4_setConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_getConfig(RBRGen4 *conn, RBRGen4Config *config,
                               RBRGen4LabelList *scheduleList);

/**
 * \brief Set the schedules in a configuration.
 *
 * Sends `schedulelist`, the command's only parameter. An empty
 * \a scheduleList sends `none`.
 *
 * \note Issues the `config <config_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] config the configuration to write, selected by its label
 * \param [in] scheduleList the schedules to put in the configuration
 * \return #RBRGEN4_SUCCESS when the configuration is written
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when the configuration cannot be written, or
 *                                      another hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty,
 *                                                    \a scheduleList is
 *                                                    `NULL`, its count does
 *                                                    not fit its array, or a
 *                                                    schedule label is empty
 * \return #RBRGEN4_BUFFER_TOO_SMALL when the list does not fit the
 *                                            command
 * \see RBRGen4_getConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_setConfig(RBRGen4 *conn, const RBRGen4Config *config,
                               const RBRGen4LabelList *scheduleList);

/**
 * \brief `config` command parameters. The `list` is stored in a user provided
 * buffer (#pool).
 *
 * \see RBRGen4_getConfigPool()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
typedef struct RBRGen4ConfigPool {
    /** \brief The number of configurations #pool can hold. */
    int32_t size;

    /**
     * \brief The number of configurations defined on the instrument.
     *
     * \warning This field will be larger than #size when
     * #RBRGEN4_TRUNCATED is returned by the getter. Care should be
     * taken to avoid out-of-bounds access when iterating over #pool.
     */
    int32_t count;

    /**
     * \brief The maximum number of configurations that can exist on the
     * instrument.
     */
    int32_t maxCount;

    /** \brief User provided buffer of the configurations defined. */
    RBRGen4Config *pool;
} RBRGen4ConfigPool;

/**
 * \brief Populate the pool of the instrument's configurations.
 *
 * Only the labels are reported; read a configuration's parameters with
 * RBRGen4_getConfig().
 *
 * \note Issues the `config` command.
 *
 * \param [in] conn the instrument connection
 * \param [in,out] configPool the configurations defined, labels only
 * \return #RBRGEN4_SUCCESS when the configurations are read
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_TRUNCATED when \a configPool cannot hold every
 *                                      reported configuration; the first
 *                                      `size` are stored, and `count` is set
 *                                      to the value reported by the instrument
 *                                      which WILL exceed `size`
 * \see RBRGen4_getConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_getConfigPool(RBRGen4 *conn, RBRGen4ConfigPool *configPool);

/**
 * \brief Create an empty configuration.
 *
 * Add schedules with RBRGen4_setConfig().
 *
 * \note Issues the `config create <config_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label to give the new configuration
 * \return #RBRGEN4_SUCCESS when the configuration is created
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when it cannot be created, or another
 *                                      hardware error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/13830131/Parameter+naming+constraints
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_createConfig(RBRGen4 *conn, const char *label);

/**
 * \brief Delete a configuration.
 *
 * \note Issues the `config delete <config_label>` command.
 *
 * \param [in] conn the instrument connection
 * \param [in] label the label of the configuration to delete
 * \return #RBRGEN4_SUCCESS when the configuration is deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \return #RBRGEN4_HARDWARE_ERROR when it does not exist, or another hardware
 *                                      error occurs
 * \return #RBRGEN4_INVALID_PARAMETER_VALUE when the label is empty
 * \see RBRGen4_deleteConfigAll()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_deleteConfig(RBRGen4 *conn, const char *label);

/**
 * \brief Delete every configuration.
 *
 * \note Issues the `config delete all` command.
 *
 * \param [in] conn the instrument connection
 * \return #RBRGEN4_SUCCESS when the configurations are deleted
 * \return #RBRGEN4_TIMEOUT when a timeout occurs
 * \return #RBRGEN4_CALLBACK_ERROR returned by a callback
 * \see RBRGen4_deleteConfig()
 * \see https://docs-rbr.atlassian.net/wiki/spaces/GEN4CR/pages/48955633/config
 */
RBRGen4Error RBRGen4_deleteConfigAll(RBRGen4 *conn);

#ifdef __cplusplus
}
#endif

#endif /* LIBRBR_RBRGEN4CONFIGURATION_H */
