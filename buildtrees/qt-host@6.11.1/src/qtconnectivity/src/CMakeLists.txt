# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Evaluate features to decide what to build.
# The config files will be written in the src/global module.
qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/../configure.cmake")

add_subdirectory(global)
if(QT_FEATURE_bluetooth)
    add_subdirectory(bluetooth)
endif()
if(QT_FEATURE_nfc)
    add_subdirectory(nfc)
endif()
if(ANDROID)
    add_subdirectory(android)
endif()

if(QT_FEATURE_bluez)
    add_subdirectory(tools/sdpscanner)
endif()
