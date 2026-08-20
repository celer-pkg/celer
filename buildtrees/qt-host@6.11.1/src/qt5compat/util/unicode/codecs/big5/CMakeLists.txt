# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from big5.pro.

#####################################################################
## big5 Binary:
#####################################################################

qt_internal_add_executable(big5
    GUI
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
)

# Resources:
set(big5_resource_files
    "BIG5"
)

qt_internal_add_resource(big5 "big5"
    PREFIX
        "/"
    FILES
        ${big5_resource_files}
)

