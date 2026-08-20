// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: buttonWithIcon
    width: 120
    height: 80
    state: "state_state_Idle"
    property string iconState: "state_icon_SkySunrise"
    property string buttonName: "Sunrise"

    required property QtObject buttonTabs

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 120
        height: 80
        color: button.pressed ? "#B8B8B9" : (button.hovered ? "#5C5C5D" : (button.checked ? "#3D3D3E" : "#222222"))
        radius: 8
    }

    IconsSkylight {
        id: iconsSkylight
        x: 36
        y: 16
        width: 48
        height: 48
        clip: true
        state: parent.iconState
    }

    Button {
        id: button
        opacity: 0
        text: qsTr("")
        anchors.fill: parent
        highlighted: false
        checkable: true
        display: AbstractButton.TextOnly
        checked: (buttonWithIcon.buttonTabs as ButtonTabs).activeBtn === buttonWithIcon.buttonName
        Connections {
            target: button
            function onClicked() {
                (buttonWithIcon.buttonTabs as ButtonTabs).activeBtn = buttonWithIcon.buttonName
            }
        }
    }
    states: [
        State {
            name: "state_state_Idle"
        },
        State {
            name: "state_state_Pressed"
            extend: "state_state_Idle"

            PropertyChanges {
                target: bg
                color: "#b8b8b9"
            }
        },
        State {
            name: "state_state_Hover"
            extend: "state_state_Idle"

            PropertyChanges {
                target: bg
                color: "#5c5c5d"
            }
        },
        State {
            name: "state_state_Active"
            extend: "state_state_Idle"

            PropertyChanges {
                target: bg
                color: "#3d3d3e"
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;uuid:"daca4806-5c88-522d-b47b-15bd86594c91"}D{i:1;uuid:"a14d30f6-0e64-5340-9fc4-e3bd2e100934"}
D{i:2;uuid:"e5a917fb-bf37-5bf0-ae35-642da89b0bcf"}
}
##^##*/

