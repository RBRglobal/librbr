# Copyright (c) 2025 RBR Ltd.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_LIBRBR)
    if(NOT CONFIG_LIBRBR_GEN3 AND NOT CONFIG_LIBRBR_GEN4)
        message(FATAL_ERROR
            "libRBR: at least one instrument generation must be enabled: "
            "set CONFIG_LIBRBR_GEN3=y and/or CONFIG_LIBRBR_GEN4=y")
    endif()

    zephyr_interface_library_named(librbr)

    include(${ZEPHYR_CURRENT_MODULE_DIR}/tools/version.cmake)

    librbr_version(${ZEPHYR_CURRENT_MODULE_DIR} RBR_LIB_VERSION)

    zephyr_compile_definitions(
        RBR_LIB_NAME="libRBR"
        RBR_LIB_VERSION="${RBR_LIB_VERSION}"
    )

    zephyr_system_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)

    zephyr_library_sources(${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRCommon.c)

    if(CONFIG_LIBRBR_GEN3)
        zephyr_compile_definitions(
            RBRGEN3_COMMAND_BUFFER_MAX=CONFIG_RBRGEN3_COMMAND_BUFFER_MAX
            RBRGEN3_RESPONSE_BUFFER_MAX=CONFIG_RBRGEN3_RESPONSE_BUFFER_MAX
            RBRGEN3_CHANNEL_MAX=CONFIG_RBRGEN3_CHANNEL_MAX

            RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX=CONFIG_RBRGEN3_CALIBRATION_C_COEFFICIENT_MAX
            RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX=CONFIG_RBRGEN3_CALIBRATION_X_COEFFICIENT_MAX
            RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX=CONFIG_RBRGEN3_CALIBRATION_N_COEFFICIENT_MAX
            RBRGEN3_CHANNEL_GAINS_MAX=CONFIG_RBRGEN3_CHANNEL_GAINS_MAX
        )

        zephyr_library_sources(
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3DynamicCorrection.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Communication.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Configuration.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Deployment.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Fetching.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Gating.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3HardwareErrors.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Internal.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Internal.h
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Memory.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Other.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Pauseresume.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Schedule.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Security.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Streaming.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Vehicle.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen3Parser.c
        )
    endif()

    if(CONFIG_LIBRBR_GEN4)
        zephyr_compile_definitions(
            RBRGEN4_CHANNEL_MAX=CONFIG_RBRGEN4_CHANNEL_MAX
        )

        zephyr_library_sources(
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Communication.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Configuration.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Deployment.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4HardwareErrors.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Instrument.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Internal.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Internal.h
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Memory.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRGen4Realtime.c
        )
    endif()
endif()
