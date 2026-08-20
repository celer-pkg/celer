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

    property real magnetRotation: 40
    property real magnetometerX: 0
    property real magnetometerY: 0
    property real magnetometerZ: 0
    property int barScaleFactor: 10000

    //! [0]
    Magnetometer {
        id: magnetometer
        active: true
        dataRate: 25
        onReadingChanged: {
            root.magnetometerX = (reading as MagnetometerReading).x
            root.magnetometerY = (reading as MagnetometerReading).y
            root.magnetometerZ = (reading as MagnetometerReading).z
            root.magnetRotation =
                ((Math.atan2(root.magnetometerX, root.magnetometerY) / Math.PI) * 180)
        }
    }
    //! [0]

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: 10

        Image {
            id: image

            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
            Layout.preferredWidth: root.imageSize * 0.9
            Layout.preferredHeight: root.imageSize * 0.9

            source: "images/magnet.svg"
            fillMode: Image.PreserveAspectFit
            rotation: root.magnetRotation
        }

        ProgressXYZBar {
            Layout.fillWidth: true
            fontSize: root.fontSize

            xText: "X: " + root.magnetometerX.toFixed(9)
            xValue: 0.5 + (root.magnetometerX * root.barScaleFactor)

            yText: "Y: " + root.magnetometerY.toFixed(9)
            yValue: 0.5 + (root.magnetometerY * root.barScaleFactor)

            zText: "Z: " + root.magnetometerZ.toFixed(9)
            zValue: 0.5 + (root.magnetometerZ * root.barScaleFactor)
        }
    }
}
