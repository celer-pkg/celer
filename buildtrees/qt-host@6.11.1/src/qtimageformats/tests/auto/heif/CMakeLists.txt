# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qheif Test:
#####################################################################

qt_internal_add_test(tst_qheif
    SOURCES
        tst_qheif.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(heif_resource_files
    "../../shared/images/heif/col320x480.heic"
    "../../shared/images/heif/newlogoCCW.heic"
    "../../shared/images/heif/primary-index-0.heic"
    "../../shared/images/heif/primary-index-1.heic"
)

qt_internal_add_resource(tst_qheif "heif"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${heif_resource_files}
)
