# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(QT_QML_GENERATE_QMLLS_INI_NO_CMAKE_CALLS ON)
qt_add_qml_module(WithoutCMakeBuilds
    URI WithoutCMakeBuilds
    QML_FILES MyItem.qml
)
