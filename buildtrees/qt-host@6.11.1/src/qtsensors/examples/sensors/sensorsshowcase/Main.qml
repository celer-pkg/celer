// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    readonly property int defaultFontSize: 22
    readonly property int imageSize: width / 2

    width: 420
    height: 760
    visible: true
    title: "Sensors Showcase"

    header : ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            ToolButton {
                id: back
                text: qsTr("Back")
                font.pixelSize: root.defaultFontSize - 4
                visible: stack.depth > 1
                onClicked: {
                    stack.pop();
                    heading.text = root.title;
                }
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: heading
                text: root.title
                font.pixelSize: root.defaultFontSize
                font.weight: Font.Medium
                verticalAlignment: Qt.AlignVCenter
                Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: 55
            }
            Item {
                visible: back.visible
                Layout.preferredWidth: back.width
            }
        }
    }

    StackView {
        id: stack

        // Pushes the object and forwards the properties
        function pusher(object : string) : void {
            // Trim the suffix and set it as new heading
            heading.text = object.split(".")[0]
            return stack.push(object, {
                fontSize: root.defaultFontSize,
                imageSize: root.imageSize
            })
        }

        anchors.fill: parent
        anchors.margins: width / 12

        initialItem: Item {
            ColumnLayout {
                id: initialItem

                anchors.fill: parent
                anchors.topMargin: 20
                anchors.bottomMargin: 20
                spacing: 5

                component CustomButton: Button {
                    highlighted: true
                    font.pixelSize: root.defaultFontSize
                    font.letterSpacing: 1.5

                    Layout.alignment: Qt.AlignCenter
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                CustomButton {
                    text: "Accelerometer"
                    onClicked: stack.pusher("Accelerometer.qml")
                    enabled: SensorSupport.hasAccelerometer()
                }
                CustomButton {
                    text: "Proximity"
                    onClicked: stack.pusher("Proximity.qml")
                    enabled: SensorSupport.hasProximity()
                }
                CustomButton {
                    text: "Compass"
                    onClicked: stack.pusher("Compass.qml")
                    enabled: SensorSupport.hasCompass()
                }
                CustomButton {
                    text: "Magnetometer"
                    onClicked: stack.pusher("Magnetometer.qml")
                    enabled: SensorSupport.hasMagnetometer()
                }
                CustomButton {
                    text: "Gyroscope"
                    onClicked: stack.pusher("Gyroscope.qml")
                    enabled: SensorSupport.hasGyroscope()
                }
            }
        }
    }

}
