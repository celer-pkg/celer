// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: buttonTabs
    width: 360
    height: 80
    state: ""
    property string activeBtn: "Sunrise"

    property int sceneState: 0

    property real sunPosition: -2

    property real newsunpos: -2
    property real oldsunpos: -2

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 360
        height: 80
        color: "#222222"
        radius: 8
    }

    ButtonWithIcon {
        id: buttonWithIcon
        buttonTabs: buttonTabs
        x: 0
        y: 0
        width: 120
        height: 80
        state: "state_state_Idle"
        iconState: "state_icon_SkySunrise"
        buttonName: "Sunrise"
    }

    ButtonWithIcon {
        id: buttonWithIcon1
        buttonTabs: buttonTabs
        x: 120
        y: 0
        width: 120
        height: 80
        state: "state_state_Idle"
        iconState: "state_icon_SkyMidday"
        buttonName: "Midday"
    }

    ButtonWithIcon {
        id: buttonWithIcon2
        buttonTabs: buttonTabs
        x: 240
        y: 0
        width: 120
        height: 80
        state: "state_state_Idle"
        iconState: "state_icon_SkySunset"
        buttonName: "Sunset"
    }

    NumberAnimation {
        id: numberAnimation
        target: buttonTabs
        property: "sunPosition"
        easing.bezierCurve: [0.445, 0.05, 0.55, 0.95, 1, 1]
        duration: 1000
        running: true
        to: 0
        from: 0
    }

    states: [
        State {
            name: "Sunrise"
            when: buttonTabs.activeBtn === "Sunrise"

            PropertyChanges {
                target: buttonTabs
                sunPosition: -5
                sceneState: 0
            }
        },

        State {
            name: "Midday"
            when: buttonTabs.activeBtn === "Midday"

            PropertyChanges {
                target: buttonTabs
                sunPosition: -120
                sceneState: 1
            }
        },

        State {
            name: "Sunset"
            when: buttonTabs.activeBtn === "Sunset"

            PropertyChanges {
                target: buttonTabs
                sunPosition: -178
                sceneState: 2
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"

            NumberAnimation {
                target: buttonTabs
                property: "sunPosition"
                duration: 1000
                easing.type: Easing.InOutSine
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;uuid:"2c5f0faa-c5fd-50a9-983c-4d5c430e52bc"}D{i:2;uuid:"3ea1e882-d922-5053-b0c7-64f1ced6fa0c"}
D{i:3;uuid:"11b40535-45b6-5496-a48c-a89a4c18cfab"}
}
##^##*/

