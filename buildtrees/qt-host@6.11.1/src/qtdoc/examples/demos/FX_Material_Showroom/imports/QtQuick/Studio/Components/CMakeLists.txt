# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_library(QQS_Components STATIC)

qt_add_qml_module(QQS_Components
    URI QtQuick.Studio.Components
    CLASS_NAME QQS_ComponentsPlugin
    QML_FILES
        ArcItem.qml
        SvgPathItem.qml
)
