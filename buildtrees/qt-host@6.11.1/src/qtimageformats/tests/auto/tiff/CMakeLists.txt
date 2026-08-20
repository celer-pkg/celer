# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qtiff Test:
#####################################################################

qt_internal_add_test(tst_qtiff
    SOURCES
        tst_qtiff.cpp
    LIBRARIES
        Qt::Gui
)

# Resources:
set(tiff_resource_files
    "../../shared/images/tiff/16bpc.tiff"
    "../../shared/images/tiff/big_16bpc.tiff"
    "../../shared/images/tiff/big_grayscale.tiff"
    "../../shared/images/tiff/big_rgb.tiff"
    "../../shared/images/tiff/big_rgb_bigendian.tiff"
    "../../shared/images/tiff/colorful.bmp"
    "../../shared/images/tiff/corrupt-data.tif"
    "../../shared/images/tiff/gray16.tiff"
    "../../shared/images/tiff/grayscale-ref.tif"
    "../../shared/images/tiff/grayscale.tif"
    "../../shared/images/tiff/image.tif"
    "../../shared/images/tiff/image_100dpi.tif"
    "../../shared/images/tiff/indexed_orientation_1.tiff"
    "../../shared/images/tiff/indexed_orientation_2.tiff"
    "../../shared/images/tiff/indexed_orientation_3.tiff"
    "../../shared/images/tiff/indexed_orientation_4.tiff"
    "../../shared/images/tiff/indexed_orientation_5.tiff"
    "../../shared/images/tiff/indexed_orientation_6.tiff"
    "../../shared/images/tiff/indexed_orientation_7.tiff"
    "../../shared/images/tiff/indexed_orientation_8.tiff"
    "../../shared/images/tiff/mono_orientation_1.tiff"
    "../../shared/images/tiff/mono_orientation_2.tiff"
    "../../shared/images/tiff/mono_orientation_3.tiff"
    "../../shared/images/tiff/mono_orientation_4.tiff"
    "../../shared/images/tiff/mono_orientation_5.tiff"
    "../../shared/images/tiff/mono_orientation_6.tiff"
    "../../shared/images/tiff/mono_orientation_7.tiff"
    "../../shared/images/tiff/mono_orientation_8.tiff"
    "../../shared/images/tiff/multipage.tiff"
    "../../shared/images/tiff/oddsize_grayscale.tiff"
    "../../shared/images/tiff/oddsize_mono.tiff"
    "../../shared/images/tiff/original_grayscale.tiff"
    "../../shared/images/tiff/original_indexed.tiff"
    "../../shared/images/tiff/original_mono.tiff"
    "../../shared/images/tiff/original_rgb.tiff"
    "../../shared/images/tiff/rgb_orientation_1.tiff"
    "../../shared/images/tiff/rgb_orientation_2.tiff"
    "../../shared/images/tiff/rgb_orientation_3.tiff"
    "../../shared/images/tiff/rgb_orientation_4.tiff"
    "../../shared/images/tiff/rgb_orientation_5.tiff"
    "../../shared/images/tiff/rgb_orientation_6.tiff"
    "../../shared/images/tiff/rgb_orientation_7.tiff"
    "../../shared/images/tiff/rgb_orientation_8.tiff"
    "../../shared/images/tiff/rgba_adobedeflate_littleendian.tif"
    "../../shared/images/tiff/rgba_lzw_littleendian.tif"
    "../../shared/images/tiff/rgba_nocompression_bigendian.tif"
    "../../shared/images/tiff/rgba_nocompression_littleendian.tif"
    "../../shared/images/tiff/rgba_packbits_littleendian.tif"
    "../../shared/images/tiff/rgba_zipdeflate_littleendian.tif"
    "../../shared/images/tiff/teapot.tiff"
    "../../shared/images/tiff/teapot_cmyk.tiff"
    "../../shared/images/tiff/tiled_grayscale.tiff"
    "../../shared/images/tiff/tiled_indexed.tiff"
    "../../shared/images/tiff/tiled_mono.tiff"
    "../../shared/images/tiff/tiled_oddsize_grayscale.tiff"
    "../../shared/images/tiff/tiled_oddsize_mono.tiff"
    "../../shared/images/tiff/tiled_rgb.tiff"
)

qt_internal_add_resource(tst_qtiff "tiff"
    PREFIX
        "/"
    BASE
        "../../shared/images"
    FILES
        ${tiff_resource_files}
)
