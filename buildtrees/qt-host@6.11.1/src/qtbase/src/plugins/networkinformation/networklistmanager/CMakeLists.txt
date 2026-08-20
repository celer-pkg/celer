# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QNLMNIPlugin
    OUTPUT_NAME qnetworklistmanager
    CLASS_NAME QNetworkListManagerNetworkInformationBackendFactory
    PLUGIN_TYPE networkinformation
    DEFAULT_IF WIN32 AND QT_FEATURE_networklistmanager
    EXCEPTIONS
    SOURCES
        qnetworklistmanagernetworkinformationbackend.cpp
        qnetworklistmanagerevents.h qnetworklistmanagerevents.cpp
    LIBRARIES
        Qt::NetworkPrivate
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QT_NO_CAST_FROM_ASCII
)

qt_internal_extend_target(QNLMNIPlugin CONDITION WIN32
    LIBRARIES
        runtimeobject
        oleaut32
)

# Don't repeat the target name in AUTOGEN_BUILD_DIR to work around issues with overlong paths.
set_property(TARGET QNLMNIPlugin PROPERTY
    AUTOGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/autogen")
