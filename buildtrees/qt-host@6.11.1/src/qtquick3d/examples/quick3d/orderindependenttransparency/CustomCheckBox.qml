// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property alias text: propText.text
    property alias checked: checkBox.checked

    width: rowLayout.width
    height: rowLayout.height

    RowLayout {
        id: rowLayout
        CheckBox {
            id: checkBox
            indicator: Rectangle {
                anchors.centerIn: parent
                implicitWidth: 20
                implicitHeight: 20
                radius: 5
                color: checkBox.pressed ? "#d0d0d0" : "#606060"
                border.color: "#848895"
                Rectangle {
                    anchors.centerIn: parent
                    implicitWidth: 12
                    implicitHeight: 12
                    radius: 4
                    color: "#ffffff"
                    border.color: "#848895"
                    visible: checkBox.checked
                }
            }
        }
        Label {
            id: propText
            color: "#f0f0f0"
            font.pointSize: AppSettings.fontSizeSmall
            Layout.minimumWidth: 150
            Layout.maximumWidth: 150
            opacity: checkBox.checked ? 1.0 : 0.5
        }
    }
}
