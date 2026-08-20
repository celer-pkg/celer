# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from assetimporters.pro.

# special case begin
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qtassetimporters-config.h"
    PRIVATE_FILE "qtassetimporters-config_p.h"
)
include("configure.cmake")
qt_feature_module_end(NO_MODULE)
# special case end

if(QT_FEATURE_quick3d_assimp)
    add_subdirectory(assimp)
endif()
