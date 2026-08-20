# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## RepParser Module:
#####################################################################

qt_internal_add_module(RepParser
    HEADER_MODULE
    NO_PRIVATE_MODULE
    PUBLIC_LIBRARIES
        Qt::Core
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_module_info(module RepParser)
qt_path_join(parser_install_dir "${QT_INSTALL_DIR}" "${INSTALL_INCLUDEDIR}" "${module}")

qt_copy_or_install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/parser.g"
        "${CMAKE_CURRENT_SOURCE_DIR}/qregexparser.h"
    DESTINATION
        "${parser_install_dir}"
)
