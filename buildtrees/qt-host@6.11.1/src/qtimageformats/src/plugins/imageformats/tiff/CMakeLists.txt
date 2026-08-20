# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QTiffPlugin Plugin:
#####################################################################

# Handle the conditional finding of either system zlib or qt zlib.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 COMPONENTS ZlibPrivate)
elseif(NOT TARGET WrapZLIB::WrapZLIB)
    qt_find_package(WrapZLIB PROVIDED_TARGETS WrapZLIB::WrapZLIB)
endif()

qt_internal_add_plugin(QTiffPlugin
    OUTPUT_NAME qtiff
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qtiffhandler.cpp qtiffhandler_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)

set_target_properties(QTiffPlugin PROPERTIES UNITY_BUILD OFF)

## Scopes:
#####################################################################

qt_internal_extend_target(QTiffPlugin CONDITION QT_FEATURE_system_tiff
    LIBRARIES
        TIFF::TIFF
)

qt_internal_extend_target(QTiffPlugin CONDITION NOT QT_FEATURE_system_tiff
    SOURCES
        ../../../3rdparty/libtiff/libtiff/tif_aux.c
        ../../../3rdparty/libtiff/libtiff/tif_close.c
        ../../../3rdparty/libtiff/libtiff/tif_codec.c
        ../../../3rdparty/libtiff/libtiff/tif_color.c
        ../../../3rdparty/libtiff/libtiff/tif_compress.c
        ../../../3rdparty/libtiff/libtiff/tif_dir.c
        ../../../3rdparty/libtiff/libtiff/tif_dirinfo.c
        ../../../3rdparty/libtiff/libtiff/tif_dirread.c
        ../../../3rdparty/libtiff/libtiff/tif_dirwrite.c
        ../../../3rdparty/libtiff/libtiff/tif_dumpmode.c
        ../../../3rdparty/libtiff/libtiff/tif_error.c
        ../../../3rdparty/libtiff/libtiff/tif_extension.c
        ../../../3rdparty/libtiff/libtiff/tif_fax3.c
        ../../../3rdparty/libtiff/libtiff/tif_fax3sm.c
        ../../../3rdparty/libtiff/libtiff/tif_flush.c
        ../../../3rdparty/libtiff/libtiff/tif_getimage.c
        ../../../3rdparty/libtiff/libtiff/tif_hash_set.c
        ../../../3rdparty/libtiff/libtiff/tif_luv.c
        ../../../3rdparty/libtiff/libtiff/tif_lzw.c
        ../../../3rdparty/libtiff/libtiff/tif_next.c
        ../../../3rdparty/libtiff/libtiff/tif_open.c
        ../../../3rdparty/libtiff/libtiff/tif_packbits.c
        ../../../3rdparty/libtiff/libtiff/tif_pixarlog.c
        ../../../3rdparty/libtiff/libtiff/tif_predict.c
        ../../../3rdparty/libtiff/libtiff/tif_print.c
        ../../../3rdparty/libtiff/libtiff/tif_read.c
        ../../../3rdparty/libtiff/libtiff/tif_strip.c
        ../../../3rdparty/libtiff/libtiff/tif_swab.c
        ../../../3rdparty/libtiff/libtiff/tif_thunder.c
        ../../../3rdparty/libtiff/libtiff/tif_tile.c
        ../../../3rdparty/libtiff/libtiff/tif_version.c
        ../../../3rdparty/libtiff/libtiff/tif_warning.c
        ../../../3rdparty/libtiff/libtiff/tif_write.c
        ../../../3rdparty/libtiff/libtiff/tif_zip.c
        ../../../3rdparty/libtiff/libtiff/t4.h
        ../../../3rdparty/libtiff/libtiff/tiff.h
        ../../../3rdparty/libtiff/libtiff/tif_config.h
        ../../../3rdparty/libtiff/libtiff/tiffconf.h
        ../../../3rdparty/libtiff/libtiff/tif_dir.h
        ../../../3rdparty/libtiff/libtiff/tiffio.h
        ../../../3rdparty/libtiff/libtiff/tif_fax3.h
        ../../../3rdparty/libtiff/libtiff/tiffiop.h
        ../../../3rdparty/libtiff/libtiff/tif_hash_set.h
        ../../../3rdparty/libtiff/libtiff/tiffvers.h
        ../../../3rdparty/libtiff/libtiff/tif_predict.h
        ../../../3rdparty/libtiff/libtiff/uvcode.h

    INCLUDE_DIRECTORIES
        ../../../3rdparty/libtiff/libtiff
    ATTRIBUTION_FILE_DIR_PATHS
        ../../../3rdparty/libtiff
    DEFINES
        LOGLUV_PUBLIC=0
)

qt_internal_extend_target(QTiffPlugin CONDITION WIN32 AND NOT QT_FEATURE_system_tiff
    SOURCES
        ../../../3rdparty/libtiff/libtiff/tif_win32.c
)

qt_internal_extend_target(QTiffPlugin CONDITION UNIX AND NOT QT_FEATURE_system_tiff
    SOURCES
        ../../../3rdparty/libtiff/libtiff/tif_unix.c
)

qt_internal_extend_target(QTiffPlugin CONDITION QT_FEATURE_system_zlib AND NOT QT_FEATURE_system_tiff
    LIBRARIES
        WrapZLIB::WrapZLIB
)

qt_internal_extend_target(QTiffPlugin CONDITION NOT QT_FEATURE_system_tiff AND NOT QT_FEATURE_system_zlib
    LIBRARIES
        Qt::ZlibPrivate
)
