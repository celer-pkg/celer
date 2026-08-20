# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::lupdate AND TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::lconvert)
    add_subdirectory(../translations translations)
else()
    message(STATUS "Skipping adding translation custom targets due to missing lupdate or lconvert.")
endif()
