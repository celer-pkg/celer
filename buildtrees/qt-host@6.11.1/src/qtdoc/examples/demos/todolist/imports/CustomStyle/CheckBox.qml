// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T

T.CheckBox {
    id: control

    implicitHeight: 24
    implicitWidth: 24

    property color controlColor: control.down ? "#17a81a" : "#21be2b"

    indicator: Rectangle {
        implicitWidth: control.implicitWidth
        implicitHeight: control.implicitHeight
        radius: 6
        color: "transparent"
        border.color: control.controlColor

        Rectangle {
            width: 14
            height: 14
            radius: 4
            anchors.centerIn: parent
            color: control.controlColor
            visible: control.checked
        }
    }
}
