# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_library(QtProtocCommon STATIC
    qtprotocdefs.h
    baseprinter.cpp baseprinter.h
    descriptorprinterbase.h
    generatorbase.cpp generatorbase.h
    generatorcommon.cpp generatorcommon.h
    options.cpp options.h
    commontemplates.cpp commontemplates.h
    utils.cpp utils.h
)

target_link_libraries(QtProtocCommon
    PUBLIC
        protobuf::libprotobuf
        protobuf::libprotoc
)

target_include_directories(QtProtocCommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
