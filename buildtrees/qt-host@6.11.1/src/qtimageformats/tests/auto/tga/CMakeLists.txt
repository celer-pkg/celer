# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qtga Test:
#####################################################################

qt_internal_add_test(tst_qtga
    SOURCES
        tst_qtga.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(tga_resource_files
    "../../shared/images/tga/test-flag.tga"
)

qt_internal_add_resource(tst_qtga "tga"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${tga_resource_files}
)
