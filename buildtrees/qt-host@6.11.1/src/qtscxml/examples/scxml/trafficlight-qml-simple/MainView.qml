// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import TrafficLightApplication

Window {
    id: root
    visible: true
    width: 100
    height: 350

    TrafficLightStateMachine {
        id: stateMachine
        running: true
    }

    Item {
        id: lights
        width: parent.width
        height: 300

        Light {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            color: "red"
            visible: stateMachine.red || stateMachine.redGoingGreen
        }

        Light {
            anchors.centerIn: parent
            color: "yellow"
            visible: stateMachine.yellow || stateMachine.blinking
        }

        Light {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            color: "green"
            visible: stateMachine.green
        }
    }

    Rectangle {
        anchors.top: lights.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        border.color: "black"

        Text {
            anchors.fill: parent
            text: stateMachine.working ? "Pause" : "Unpause"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            onClicked: stateMachine.submitEvent(stateMachine.working ? "smash" : "repair");
        }
    }
}
