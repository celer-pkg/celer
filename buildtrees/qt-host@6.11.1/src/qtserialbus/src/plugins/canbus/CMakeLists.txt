# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(virtualcan)
if(QT_FEATURE_socketcan)
    add_subdirectory(socketcan)
endif()
if(QT_FEATURE_library)
    add_subdirectory(passthrucan)
    add_subdirectory(peakcan)
    add_subdirectory(tinycan)
endif()
if(QT_FEATURE_library AND WIN32)
    add_subdirectory(systeccan)
endif()
if(QT_FEATURE_library AND WIN32 AND NOT WINRT)
    add_subdirectory(vectorcan)
endif()
