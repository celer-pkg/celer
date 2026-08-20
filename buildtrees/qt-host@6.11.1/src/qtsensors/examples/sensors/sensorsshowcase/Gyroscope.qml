// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtSensors

Item {
    id: root

    required property int fontSize
    required property int imageSize

    function resetRotations() : void
    {
        imageXRotation.angle = 0
        imageYRotation.angle = 0
        imageZRotation.angle = 0
    }

    //! [0]
    Gyroscope {
        id: gyroscope

        property var lastTimeStamp: 0
        property real x: 0
        property real y: 0
        property real z: 0

        active: true
        dataRate: 25

        onReadingChanged: {
            x = (reading as GyroscopeReading).x
            y = (reading as GyroscopeReading).y
            z = (reading as GyroscopeReading).z
            let firstCall = false
            if (lastTimeStamp == 0) {
                firstCall = true
            }
            let timeSinceLast = reading.timestamp - lastTimeStamp
            lastTimeStamp = reading.timestamp

            //Skipping the initial time jump from 0
            if (firstCall === true)
                return
            let normalizedX = x * (timeSinceLast / 1000000)
            imageXRotation.angle += normalizedX
            let normalizedY = y * (timeSinceLast / 1000000)
            imageYRotation.angle -= normalizedY
            let normalizedZ = z * (timeSinceLast / 1000000)
            imageZRotation.angle += normalizedZ
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
            Layout.fillHeight: true
            Layout.preferredWidth: root.imageSize
            fillMode: Image.PreserveAspectFit
            source: "images/qt_logo.png"

            transform: [
                Rotation {
                    id: imageXRotation

                    angle: 0
                    axis.x: 1
                    axis.y: 0
                    axis.z: 0
                    origin.x: layout.width / 2
                    origin.y: layout.height / 3
                },
                Rotation {
                    id: imageYRotation

                    angle: 0
                    axis.x: 0
                    axis.y: 1
                    axis.z: 0
                    origin.x: layout.width / 2
                    origin.y: layout.height / 3
                },
                Rotation {
                    id: imageZRotation

                    angle: 0
                    axis.x: 0
                    axis.y: 0
                    axis.z: 1
                    origin.x: layout.width / 2
                    origin.y: layout.height / 3
                }
            ]
        }

        ProgressXYZBar {
            Layout.fillWidth: true
            Layout.topMargin: 20
            fontSize: root.fontSize
            xText: "X: " + gyroscope.x.toFixed(2)
            xValue: 0.5 + (gyroscope.x / 1000)
            yText: "Y: " + gyroscope.y.toFixed(2)
            yValue: 0.5 + (gyroscope.y / 1000)
            zText: "Z: " + gyroscope.z.toFixed(2)
            zValue: 0.5 + (gyroscope.z / 1000)
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            Layout.bottomMargin: 10
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: 60
            onClicked: root.resetRotations()
            text: "Reset rotation"
        }
    }
}
