// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtSensors

Item {
    id: root

    required property int fontSize
    required property int imageSize
    property alias isActive: compass.active

    property real azimuth: 30

    Compass {
        id: compass
        active: true
        dataRate: 7
        onReadingChanged: root.azimuth = -(reading as CompassReading).azimuth
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: 10

        Image {
            id: arrow

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.imageSize * 1.25
            Layout.fillHeight: true

            source: "images/compass.svg"
            fillMode: Image.PreserveAspectFit
            rotation: root.azimuth
        }

        Rectangle {
            id: separator

            Layout.topMargin: 10
            Layout.preferredWidth: parent.width * 0.75
            Layout.preferredHeight: 1
            Layout.alignment: Qt.AlignHCenter
            color: "black"
        }

        Text {
            id: info
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 10
            text: "Azimuth: " + root.azimuth.toFixed(2) + "°"
            font.pixelSize: root.fontSize
        }
    }
}
