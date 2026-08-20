# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Tools Module:
#####################################################################

qt_internal_add_module(Connectivity
    INTERNAL_MODULE
    HEADER_MODULE
    CONFIGURE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../configure.cmake"
    MODULE_INTERFACE_NAME Connectivity # Specify the 'Connectivity' name explicitly to avoid
                                       # warning related to the expected name of internal module.
    NO_GENERATE_CPP_EXPORTS
)
