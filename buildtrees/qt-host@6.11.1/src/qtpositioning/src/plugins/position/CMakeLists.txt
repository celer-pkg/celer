# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from position.pro.

add_subdirectory(positionpoll)
if(TARGET Qt::DBus AND (FREEBSD OR LINUX OR OPENBSD OR NETBSD OR HURD))
    add_subdirectory(geoclue2)
endif()
if(QT_FEATURE_gypsy)
    add_subdirectory(gypsy)
endif()
if(QT_FEATURE_winrt_geolocation)
    add_subdirectory(winrt)
endif()
if(IOS OR MACOS)
    add_subdirectory(corelocation)
endif()
if(ANDROID)
    add_subdirectory(android)
endif()
if(WASM)
    add_subdirectory(wasm)
endif()
if(TARGET Qt::Network)
    add_subdirectory(nmea)
endif()
