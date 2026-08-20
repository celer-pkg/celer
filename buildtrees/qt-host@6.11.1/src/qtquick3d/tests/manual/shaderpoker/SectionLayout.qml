// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property string title: ""
    property bool isExpanded: true
    default property alias content: container.children
    color: "transparent"
    border.width: 2
    border.color: palette.alternateBase

    Layout.fillWidth: true
    implicitHeight: outsideLayout.implicitHeight

    ColumnLayout {
        id: outsideLayout
        width: parent.width
        Rectangle {
            id: sectionHeader
            Layout.fillWidth: true
            implicitHeight: 24
            color: palette.alternateBase

            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    fillMode: Image.PreserveAspectFit
                    sourceSize: Qt.size(titleText.implicitHeight, titleText.implicitHeight)
                    source: root.isExpanded ? "images/TreeExpanded.png" : "images/TreeUnexpanded.png"
                }

                Text {
                    id: titleText
                    Layout.fillWidth: true
                    text: root.title
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.isExpanded = !root.isExpanded
            }
        }

        GridLayout {
            id: container
            Layout.margins: root.border.width * 2
            visible: root.isExpanded
            Layout.fillWidth: true
            columns: 2
            uniformCellWidths: true
        }

    }

}
