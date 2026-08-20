# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qjp2 Test:
#####################################################################

qt_internal_add_test(tst_qjp2
    SOURCES
        tst_qjp2.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(jp2_resource_files
    "../../shared/images/jp2/logo.bmp"
    "../../shared/images/jp2/logo.jp2"
)

qt_internal_add_resource(tst_qjp2 "jp2"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${jp2_resource_files}
)
