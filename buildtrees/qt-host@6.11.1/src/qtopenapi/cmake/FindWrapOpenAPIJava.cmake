# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET WrapOpenAPIJava::WrapOpenAPIJava)
    set(WrapOpenAPIJava_FOUND TRUE)
    return()
endif()

find_package(Java COMPONENTS Runtime)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapOpenAPIJava
    REQUIRED_VARS
        Java_JAVA_EXECUTABLE
)

if(WrapOpenAPIJava_FOUND)
    add_library(WrapOpenAPIJava::WrapOpenAPIJava INTERFACE IMPORTED)
    set_target_properties(WrapOpenAPIJava::WrapOpenAPIJava PROPERTIES
        INTERFACE_OPENAPI_JAVA_RUNTIME_PATH "${Java_JAVA_EXECUTABLE}"
    )
endif()

