// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

pragma ComponentBehavior: Bound

ApplicationWindow {
    id: window
    visible: true
    width: 480
    height: 640
    title: "QML Flexbox Layout"

    ColumnLayout {
        id: content
        property int count: 10
        anchors.fill: parent
        Rectangle {
            id: mainContent
            Layout.preferredWidth: col.implicitWidth
            Layout.preferredHeight: col.implicitHeight
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            clip: true
            border.width: 1
            border.color: "gray"
            color: "transparent"
            ColumnLayout {
                id: col
                anchors.fill: parent
                FlexboxLayout {
                    id: flex
                    wrap: FlexboxLayout.Wrap
                    gap: 10
                    justifyContent: FlexboxLayout.JustifyCenter
                    Layout.margins: 20
                    Repeater {
                        model: content.count
                        delegate: Rectangle {
                            id: rect
                            property int size: Math.floor(Math.random() * (140 - 85 + 1)) + 85
                            Layout.preferredWidth: size
                            Layout.preferredHeight: size
                            radius: 10
                            color: "#328930"
                            Text {
                                anchors.centerIn: parent
                                text: "Qt"
                                font.pixelSize: 20
                            }
                        }
                    }
                }
            }
        }
        Text {
            Layout.margins: 20
            Layout.alignment: Qt.AlignHCenter
            text: "Resize the window to observe how the flex layout adapts."
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }
    }
}
