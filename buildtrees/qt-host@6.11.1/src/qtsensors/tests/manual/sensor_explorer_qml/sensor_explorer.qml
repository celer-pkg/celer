// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import SensorModels

Window {
    id: window
    width: 400
    height: 600

    AvailableSensorsModel {
        id: availableSensorsModel
    }

    ColumnLayout {

        GroupBox {
            id: availableSensorsModelGroup
            title: qsTr("Available Sensors")
            Layout.preferredWidth: window.width - 4 // 4 = 2x2 margins
            Layout.preferredHeight: window.height * 0.4
            Layout.margins: 2

            ListView {
                id: sensorsView
                anchors.fill: parent
                currentIndex: -1 // no initial selection
                spacing: 1
                clip: true
                model: availableSensorsModel
                delegate: Item {
                    id: sensorRow
                    width: sensorsView.width
                    height: 30
                    property color rowColor: {
                        if (sensorsView.currentIndex == index)
                            return "lightsteelblue" // highlight
                        return (index % 2 == 0) ? "#CCCCCC" : "#AAAAAA"
                    }
                    RowLayout {
                        spacing: 1
                        anchors.fill: parent
                        Rectangle {
                            color: sensorRow.rowColor
                            Layout.preferredWidth:  sensorRow.width * 0.8
                            Layout.preferredHeight: sensorRow.height
                            Text {
                                anchors.centerIn: parent
                                text: display.type + "::" + display.identifier
                            }
                        }
                        Rectangle {
                            color: sensorRow.rowColor
                            Layout.preferredWidth:  sensorRow.width * 0.2
                            Layout.preferredHeight: sensorRow.height
                            Text {
                                anchors.centerIn: parent
                                text: display.active ? qsTr("Active") : qsTr("Inactive")
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: sensorsView.currentIndex = index
                    }
                }
            }
        }

        SensorPropertyModel {
            id: propertyModel
            sensor: availableSensorsModel.get(sensorsView.currentIndex)
        }

        Button {
            id: activateButton
            Layout.preferredHeight: 30
            Layout.alignment: Qt.AlignCenter
            enabled: propertyModel.sensor
            text: !propertyModel.sensor ? qsTr("Select sensor")
                                        : (propertyModel.sensor.active ? qsTr("Deactivate sensor")
                                                                       : qsTr("Activate sensor"))
            onClicked: propertyModel.sensor.active = !propertyModel.sensor.active
        }

        GroupBox {
            title: qsTr("Selected sensor's properties")
            Layout.preferredWidth: window.width  - 4 // 4 = 2x2 margins
            Layout.preferredHeight: window.height * 0.55 - activateButton.height
            Layout.margins: 2
            enabled: sensorsView.currentIndex != -1

            TableView {
                id: propertyView
                anchors.fill: parent
                model: propertyModel
                columnSpacing: 1
                rowSpacing: 1
                boundsMovement: Flickable.StopAtBounds
                clip: true

                delegate: Rectangle {
                    implicitHeight: 30
                    implicitWidth: propertyView.width * 0.5
                    color: (model.row % 2 == 0) ? "#CCCCCC" : "#AAAAAA"
                    Text {
                        anchors.centerIn: parent
                        text: display
                    }
                }
            }
        }
    }
}
