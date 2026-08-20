# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs

# input tiff
set(INPUT_tiff "undefined" CACHE STRING "")
set_property(CACHE INPUT_tiff PROPERTY STRINGS undefined no qt system)

# input webp
set(INPUT_webp "undefined" CACHE STRING "")
set_property(CACHE INPUT_webp PROPERTY STRINGS undefined no qt system)



#### Libraries

qt_find_package(WrapJasper PROVIDED_TARGETS WrapJasper::WrapJasper MODULE_NAME imageformats
    QMAKE_LIB jasper
    VCPKG_PORT jasper
    VCPKG_ADD_TO_FEATURE jasper
)
qt_find_package(TIFF PROVIDED_TARGETS TIFF::TIFF MODULE_NAME imageformats QMAKE_LIB tiff
    VCPKG_PORT tiff
    VCPKG_ADD_TO_FEATURE tiff
    VCPKG_DEFAULT_FEATURES OFF
)
# Threads::Threads might be brought in via a top-level CMakeLists.txt find_package dependency
# in which case if the system WebpConfig.cmake depends Threads, it shouldn't try to promote it to
# global to avoid a 'global promotion of a target in a different subdirectory' error.
if(TARGET Threads::Threads)
    qt_internal_disable_find_package_global_promotion(Threads::Threads)
endif()
qt_find_package(WrapWebP PROVIDED_TARGETS WrapWebP::WrapWebP MODULE_NAME imageformats
    QMAKE_LIB webp
    VCPKG_PORT libwebp
    VCPKG_ADD_TO_FEATURE webp
)
qt_find_package(Libmng PROVIDED_TARGETS Libmng::Libmng MODULE_NAME imageformats QMAKE_LIB mng)


#### Tests



#### Features

qt_feature("jasper" PRIVATE
    LABEL "JasPer"
    CONDITION QT_FEATURE_imageformatplugin AND WrapJasper_FOUND
    DISABLE INPUT_jasper STREQUAL 'no'
)
qt_feature_definition("jasper" "QT_NO_IMAGEFORMAT_JASPER" NEGATE)
qt_feature("mng" PRIVATE
    LABEL "MNG"
    CONDITION Libmng_FOUND
    DISABLE INPUT_mng STREQUAL 'no'
)
qt_feature("tiff" PRIVATE
    LABEL "TIFF"
    CONDITION QT_FEATURE_imageformatplugin
    DISABLE INPUT_tiff STREQUAL 'no'
)
qt_feature("system-tiff" PRIVATE SYSTEM_LIBRARY
    LABEL "  Using system libtiff"
    CONDITION QT_FEATURE_tiff AND TIFF_FOUND
    ENABLE INPUT_tiff STREQUAL 'system'
    DISABLE INPUT_tiff STREQUAL 'qt'
)
qt_feature("webp" PRIVATE
    LABEL "WEBP"
    CONDITION QT_FEATURE_imageformatplugin
    DISABLE INPUT_webp STREQUAL 'no'
)
qt_feature("system-webp" PRIVATE SYSTEM_LIBRARY
    LABEL "  Using system libwebp"
    CONDITION QT_FEATURE_webp AND WrapWebP_FOUND
    ENABLE INPUT_webp STREQUAL 'system'
    DISABLE INPUT_webp STREQUAL 'qt'
)
qt_configure_add_summary_section(NAME "Further Image Formats")
qt_configure_add_summary_entry(ARGS "jasper")
qt_configure_add_summary_entry(ARGS "mng")
qt_configure_add_summary_entry(ARGS "tiff")
qt_configure_add_summary_entry(ARGS "system-tiff")
qt_configure_add_summary_entry(ARGS "webp")
qt_configure_add_summary_entry(ARGS "system-webp")
qt_configure_end_summary_section() # end of "Further Image Formats" section
