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

    if(CONFIG_LIBRBR_GEN3)
        zephyr_compile_definitions(
            RBRINSTRUMENT_COMMAND_BUFFER_MAX=CONFIG_RBRINSTRUMENT_COMMAND_BUFFER_MAX
            RBRINSTRUMENT_RESPONSE_BUFFER_MAX=CONFIG_RBRINSTRUMENT_RESPONSE_BUFFER_MAX
            RBRINSTRUMENT_CHANNEL_MAX=CONFIG_RBRINSTRUMENT_CHANNEL_MAX

            RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX=CONFIG_RBRINSTRUMENT_CALIBRATION_C_COEFFICIENT_MAX
            RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX=CONFIG_RBRINSTRUMENT_CALIBRATION_X_COEFFICIENT_MAX
            RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX=CONFIG_RBRINSTRUMENT_CALIBRATION_N_COEFFICIENT_MAX
            RBRINSTRUMENT_CHANNEL_GAINS_MAX=CONFIG_RBRINSTRUMENT_CHANNEL_GAINS_MAX
        )

        zephyr_library_sources(
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRDynamicCorrection.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrument.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentCommunication.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentConfiguration.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentDeployment.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentFetching.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGating.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentHardwareErrors.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentInternal.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentInternal.h
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentMemory.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentOther.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentPauseresume.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentSchedule.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentSecurity.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentStreaming.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentVehicle.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRParser.c
        )
    endif()

    if(CONFIG_LIBRBR_GEN4)
        zephyr_compile_definitions(
            RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX=CONFIG_RBRINSTRUMENTGEN4_COMMAND_BUFFER_MAX
            RBRINSTRUMENTGEN4_RESPONSE_BUFFER_MAX=CONFIG_RBRINSTRUMENTGEN4_RESPONSE_BUFFER_MAX
            RBRINSTRUMENTGEN4_CHANNEL_MAX=CONFIG_RBRINSTRUMENTGEN4_CHANNEL_MAX
        )

        zephyr_library_sources(
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRDynamicCorrectionGen4.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Communication.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Configuration.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Deployment.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4HardwareErrors.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Instrument.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Internal.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Internal.h
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Memory.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Realtime.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRInstrumentGen4Streaming.c
            ${ZEPHYR_CURRENT_MODULE_DIR}/src/RBRParserGen4.c
        )
    endif()
endif()
