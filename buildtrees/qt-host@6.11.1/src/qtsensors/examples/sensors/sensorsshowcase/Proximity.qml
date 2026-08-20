// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtSensors

Item {
    id: root

    required property int imageSize
    required property int fontSize

    property bool near: false

    ProximitySensor {
        id: proximity
        onReadingChanged: root.near = (reading as ProximityReading).near
        active: true
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: 10

        Image {
            id: image

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.near ? root.imageSize : root.imageSize * 0.75
            Layout.fillHeight: true

            source: "images/qt_logo.png"
            fillMode: Image.PreserveAspectFit
        }

        Rectangle {
            id: separator
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Layout.preferredWidth: parent.width * 0.75
            Layout.preferredHeight: 1
            Layout.alignment: Qt.AlignHCenter
            color: "black"
        }

        Text {
            Layout.fillHeight: true
            font.pixelSize: root.fontSize
            text: "Near: " + root.near
        }
    }
}
