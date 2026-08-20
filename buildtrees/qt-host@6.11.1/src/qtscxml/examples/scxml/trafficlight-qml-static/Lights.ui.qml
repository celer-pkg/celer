// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import TrafficLightApplication

Image {
    id: lights

    property alias button: button
    property TrafficLightStateMachine stateMachine

    source: "background.png"

    Column {
        y: 40
        spacing: 27
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: redLight
            opacity: 0.2
            source: "red.png"
        }

        Image {
            id: yellowLight
            opacity: 0.2
            source: "yellow.png"
        }

        Image {
            id: greenLight
            opacity: 0.2
            source: "green.png"
        }
    }

    Button {
        id: button

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        source: "pause.png"
    }

    states: [
        State {
            name: "Red"
            when: lights.stateMachine.red

            PropertyChanges { redLight.opacity: 1 }
        },
        State {
            name: "RedGoingGreen"
            when: lights.stateMachine.redGoingGreen

            PropertyChanges { redLight.opacity: 1 }
            PropertyChanges { yellowLight.opacity: 1 }
        },
        State {
            name: "Yellow"
            when: lights.stateMachine.yellow || lights.stateMachine.blinking

            PropertyChanges { yellowLight.opacity: 1 }
        },
        State {
            name: "Green"
            when: lights.stateMachine.green

            PropertyChanges { greenLight.opacity: 1 }
        }
    ]
}
