# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# special case begin
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qtquick3dxr-config.h"
    PRIVATE_FILE "qtquick3dxr-config_p.h"
)
include("configure.cmake")
qt_feature_module_end(NO_MODULE)

if(QT_FEATURE_quick3dxr_openxr OR VISIONOS)
    add_subdirectory(quick3dxr)
endif()
