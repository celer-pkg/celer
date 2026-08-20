// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "white"
    border.width: model.isLoadingElement ? 0 : 1
    border.color: "black"
    implicitHeight: 50
    RowLayout {
        anchors.fill: parent
        spacing: 5
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            implicitHeight: 30
            implicitWidth: implicitHeight
            visible: model.isLoadingElement
        }
        Label {
            font.pixelSize: 30
            font.bold: true
            text: model.number
            visible: !model.isLoadingElement
        }
        ColumnLayout {
            implicitHeight: 40
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 3
            visible: !model.isLoadingElement
            Label {
                font.pixelSize: 16
                text: model.title
            }
            Label {
                font.pixelSize: 14
                text: model.subtitle
            }
        }
    }
}
