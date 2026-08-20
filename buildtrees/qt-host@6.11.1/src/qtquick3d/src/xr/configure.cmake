# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Inputs

# input quick3d-openxr
set(INPUT_openxr "undefined" CACHE STRING "")
set_property(CACHE INPUT_openxr PROPERTY STRINGS undefined no qt system)

#### Libraries

if(MACOS)
    set(openxr_version 1.1.40)
else()
    set(openxr_version 1.0.29)
endif()

qt_find_package(WrapSystemOpenXR ${openxr_version}
    PROVIDED_TARGETS WrapSystemOpenXR::WrapSystemOpenXR
    MODULE_NAME quick3dxr
    QMAKE_LIB quick3dxr_openxr
    VCPKG_PORT openxr-loader
    VCPKG_PLATFORM "windows | linux | osx | android"
    VCPKG_ADD_TO_FEATURE quick3dxr-openxr
)

#### Tests

#### Features

qt_feature("quick3dxr-openxr" PRIVATE
    LABEL "OpenXR"
    DISABLE INPUT_openxr STREQUAL 'no'
    AUTODETECT ANDROID OR LINUX OR WIN32 OR MACOS
)
qt_feature_definition("quick3dxr-openxr" "QT_NO_QUICK3DXR_OPENXR" NEGATE)
qt_feature("system-openxr" PRIVATE
    LABEL "  Using system OpenXR"
    CONDITION QT_FEATURE_quick3dxr_openxr AND WrapSystemOpenXR_FOUND
    ENABLE INPUT_openxr STREQUAL 'system'
    DISABLE INPUT_openxr STREQUAL 'qt'
)

qt_configure_add_summary_section(NAME "QtQuick3D XR")
qt_configure_add_summary_entry(ARGS "quick3dxr-openxr")
qt_configure_add_summary_entry(ARGS "system-openxr")
qt_configure_end_summary_section()
