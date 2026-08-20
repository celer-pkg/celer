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

    //! [0]
    Accelerometer {
        id: accelerometer

        property real x: 0
        property real y: 0
        property real z: 0

        active: true
        dataRate: 25

        onReadingChanged: {
            x = (reading as AccelerometerReading).x
            y = (reading as AccelerometerReading).y
            z = (reading as AccelerometerReading).z
            imageTranslation.x = -x * 10
            imageTranslation.y = y * 10
        }
    }
    //! [0]
    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: 10

        Image {
            id: image

            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: root.imageSize
            Layout.preferredWidth: root.imageSize
            fillMode: Image.PreserveAspectFit
            source: "images/qt_logo.png"

            transform: [
                Translate {
                    id: imageTranslation

                    x: 0
                    y: 0
                }
            ]
        }

        ProgressXYZBar {
            Layout.fillWidth: true
            fontSize: root.fontSize
            xText: "X: " + accelerometer.x.toFixed(2)
            xValue: 0.5 + (accelerometer.x / 100)
            yText: "Y: " + accelerometer.y.toFixed(2)
            yValue: 0.5 + (accelerometer.y / 100)
            zText: "Z: " + accelerometer.z.toFixed(2)
            zValue: 0.5 + (accelerometer.z / 100)
        }
    }
}
