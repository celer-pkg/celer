// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.VectorImage
import QtQuick.Controls.Basic

import qtgrpc.examples.vehicle

ApplicationWindow {
    id: root

    property int speed: -1
    property int rpm: -1
    property int totalDistance: -1
    property int traveledDistance: -1
    property int estimatedAutonomy: -1
    property string directionImageSource: ""
    property string street: ""
    property string vehicleErrors: ""
    property string navigationErrors: ""

    width: 1200
    height: 500
    visible: true
    title: qsTr("Vehicle Qt GRPC example")

//! [Connections]
    Connections {
        target: VehicleManager

        // This slot will be executed when the VehicleManager::totalDistanceChanged
        // signal is emitted
        function onTotalDistanceChanged(distance: int): void {
            root.totalDistance = distance;
        }

        function onSpeedChanged(speed: int): void {
            root.speed = speed;
        }

        function onRpmChanged(rpm: int): void {
            root.rpm = rpm;
        }

        function onTraveledDistanceChanged(distance: int): void {
            root.traveledDistance = distance;
        }

        function onDirectionChanged(direction: int): void {
            if (direction == VehicleManager.RIGHT) {
                root.directionImageSource = "qrc:/direction_right.svg";
            } else if (direction == VehicleManager.LEFT) {
                root.directionImageSource =  "qrc:/direction_left.svg";
            } else if (direction == VehicleManager.STRAIGHT) {
                root.directionImageSource =  "qrc:/direction_straight.svg";
            } else {
                root.directionImageSource =  "";
            }
        }
//! [Connections]
        function onStreetChanged(street: string): void {
            root.street = street;
        }

        function onAutonomyChanged(autonomy: int): void {
            root.estimatedAutonomy = autonomy;
        }

        function onVehicleErrorsChanged(vehicleErrors: string): void {
            root.vehicleErrors = vehicleErrors;
        }

        function onNavigationErrorsChanged(navigationErrors: string): void {
            root.navigationErrors = navigationErrors;
        }
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#160404"
    }

    // Information screen
    GridLayout {
        anchors.fill: parent
        anchors.margins: 40
        visible: !(root.vehicleErrors && root.navigationErrors)
        columnSpacing: 20
        rowSpacing: 10
        columns: 2
        uniformCellWidths: true

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            StyledText {
                id: speedText
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: root.speed == -1 ? "-" : root.speed
                font.pointSize: 90
            }

            StyledText {
                id: speedUnitText
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                text: "Km/h"
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            VectorImage {
                id: directionImage
                source: root.directionImageSource

                width: 100
                height: 100

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
            }

            Rectangle {
                visible: root.directionImageSource
                anchors.fill: directionImage
                color: "#363636"
                radius: 10
                z: -1
            }

            StyledText {
                anchors.right: parent.right
                anchors.bottom: streetText.top
                font.pointSize: 24
                horizontalAlignment: Text.AlignRight
                text: {
                    if (root.totalDistance == -1 || root.traveledDistance == -1) {
                        return "- km";
                    }

                    let remainingDistance = root.totalDistance - root.traveledDistance;

                    if (remainingDistance > 1000) {
                        return `${Number(remainingDistance * 0.001).toFixed(2)} km`;
                    }
                    return `${remainingDistance} m`
                }
            }

            StyledText {
                id: streetText

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                horizontalAlignment: Text.AlignRight
                color: "#828284"
                text: root.street
            }
        }

        StyledProgressBar {
            Layout.fillWidth: true
            value: root.speed
            to: 200
            activeColor: "#04e2ed"
            bgColor: "#023061"
        }

        StyledProgressBar {
            Layout.fillWidth: true
            value: root.traveledDistance
            to: root.totalDistance != -1 ? root.totalDistance : 0
            activeColor: "#dde90000"
            bgColor: "#860000"
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            StyledText {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: root.rpm == -1 ? "-" : root.rpm
                font.pointSize: 60
            }

            StyledText {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                text: "rpm"
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            VectorImage {
                id: fuelIcon
                width: 28
                height: 28
                source: "qrc:/fuel_icon.svg"
                anchors.bottom: parent.bottom
                anchors.right: autonomyText.left
                anchors.rightMargin: 12
            }

            StyledText {
                id: autonomyText
                text: `${root.estimatedAutonomy == -1 ? "-" : root.estimatedAutonomy} km`
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }
        }

        StyledProgressBar {
            Layout.fillWidth: true
            value: root.rpm
            to: 9000
            activeColor: "#f8c607"
            bgColor: "#5f3f04"
        }

        StyledProgressBar {
            Layout.fillWidth: true
            value: root.estimatedAutonomy
            to: 250
            activeColor: "#05c848"
            bgColor: "#03511f"
        }
    }

    // No connection error screen
    ColumnLayout {
        anchors.centerIn: parent
        visible: root.vehicleErrors && root.navigationErrors

        StyledText {
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: 14
            text: qsTr("Please, start vehicle server. Then, press try again.")
        }

        Button {
            id: runButton
            property string baseColor: "white"
            property string clickedColor: "#828284"
            property string hoverColor: "#a9a9a9"

            Layout.alignment: Qt.AlignHCenter

            background: Rectangle {
                border.color: runButton.down ?
                    runButton.clickedColor
                    : (runButton.hovered ? runButton.hoverColor : runButton.baseColor)
                border.width: 2
                radius: 2
                color: "transparent"
            }

            contentItem: Text {
                text: qsTr("Try again")
                font.pointSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: runButton.down ?
                    runButton.clickedColor
                    : (runButton.hovered ? runButton.hoverColor : runButton.baseColor)
            }

            onClicked: {
                root.vehicleErrors = "";
                root.navigationErrors = "";
                VehicleManager.restart();
            }
        }
    }
}
