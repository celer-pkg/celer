# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(OpenApiCommon
    NO_PRIVATE_MODULE
    NO_GENERATE_CPP_EXPORTS
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CONTEXTLESS_CONNECT
        QT_NO_USING_NAMESPACE
        QT_USE_NODISCARD_FILE_OPEN
    LIBRARIES
        Qt::CorePrivate
        Qt::NetworkPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
)

# Handle the conditional finding of either system zlib or qt zlib.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 COMPONENTS ZlibPrivate)
    qt_internal_extend_target(OpenApiCommon LIBRARIES Qt::ZlibPrivate)
elseif(NOT TARGET WrapZLIB::WrapZLIB)
    qt_find_package(WrapZLIB PROVIDED_TARGETS WrapZLIB::WrapZLIB)
endif()

if(TARGET WrapZLIB::WrapZLIB)
    qt_internal_extend_target(OpenApiCommon LIBRARIES WrapZLIB::WrapZLIB)
endif()

qt6_add_openapi_client(OpenApiCommon
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/qtopenapi-common.yaml
    __QT_INTERNAL_GENERATE_COMMON_LIBRARY_TARGET
)
