# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qdds Test:
#####################################################################

qt_internal_add_test(tst_qdds
    SOURCES
        tst_qdds.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(dds_resource_files
    "../../shared/images/dds/A16B16G16R16.dds"
    "../../shared/images/dds/A16B16G16R16F.dds"
    "../../shared/images/dds/A1R5G5B5.dds"
    "../../shared/images/dds/A2B10G10R10.dds"
    "../../shared/images/dds/A2R10G10B10.dds"
    "../../shared/images/dds/A2W10V10U10.dds"
    "../../shared/images/dds/A32B32G32R32F.dds"
    "../../shared/images/dds/A4L4.dds"
    "../../shared/images/dds/A4R4G4B4.dds"
    "../../shared/images/dds/A8.dds"
    "../../shared/images/dds/A8B8G8R8.dds"
    "../../shared/images/dds/A8L8.dds"
    "../../shared/images/dds/A8R3G3B2.dds"
    "../../shared/images/dds/A8R8G8B8.2.dds"
    "../../shared/images/dds/A8R8G8B8.dds"
    "../../shared/images/dds/ATI2.dds"
    "../../shared/images/dds/CxV8U8.dds"
    "../../shared/images/dds/DXT1.dds"
    "../../shared/images/dds/DXT2.dds"
    "../../shared/images/dds/DXT3.dds"
    "../../shared/images/dds/DXT4.dds"
    "../../shared/images/dds/DXT5.dds"
    "../../shared/images/dds/G16R16.dds"
    "../../shared/images/dds/G16R16F.dds"
    "../../shared/images/dds/G32R32F.dds"
    "../../shared/images/dds/G8R8_G8B8.dds"
    "../../shared/images/dds/L16.dds"
    "../../shared/images/dds/L6V5U5.dds"
    "../../shared/images/dds/L8.dds"
    "../../shared/images/dds/P4.dds"
    "../../shared/images/dds/P8.dds"
    "../../shared/images/dds/Q16W16V16U16.dds"
    "../../shared/images/dds/Q8W8V8U8.dds"
    "../../shared/images/dds/R16F.dds"
    "../../shared/images/dds/R32F.dds"
    "../../shared/images/dds/R3G3B2.dds"
    "../../shared/images/dds/R5G6B5.dds"
    "../../shared/images/dds/R8G8B8.dds"
    "../../shared/images/dds/R8G8_B8G8.dds"
    "../../shared/images/dds/RXGB.dds"
    "../../shared/images/dds/UYVY.dds"
    "../../shared/images/dds/V16U16.dds"
    "../../shared/images/dds/V8U8.dds"
    "../../shared/images/dds/X1R5G5B5.dds"
    "../../shared/images/dds/X4R4G4B4.dds"
    "../../shared/images/dds/X8B8G8R8.dds"
    "../../shared/images/dds/X8L8V8U8.dds"
    "../../shared/images/dds/X8R8G8B8.dds"
    "../../shared/images/dds/YUY2.dds"
    "../../shared/images/dds/mipmaps.dds"
)

qt_internal_add_resource(tst_qdds "dds"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${dds_resource_files}
)
