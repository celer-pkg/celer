// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: settingsbutton
    width: 311
    height: 80
    property alias open_setting_panelText: open_setting_panel.text
    state: "state_state_Idle"

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 311
        height: 80
        color: "#222222"
        radius: 8
    }

    Text {
        id: open_setting_panel
        x: 28
        y: 26
        width: 180
        height: 28
        color: "#ffffff"
        text: qsTr("Open setting panel")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        font.family: "Archivo"
        font.weight: Font.Bold
    }

    IconsFunctional {
        id: iconsFunctional
        x: 235
        y: 16
        width: 48
        height: 48
        clip: true
        state: "state_icon_ArrowRight"
    }
    states: [
        State {
            name: "state_state_Idle"
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
            name: "state_state_Pressed"
            extend: "state_state_Idle"

            PropertyChanges {
                target: bg
                color: "#b8b8b9"
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:80;width:311}D{i:1;uuid:"c7c4867f-637a-5518-aceb-7f1dd2290416"}D{i:2;uuid:"5266f2b1-d65a-5433-a9f7-76f929ec337c"}
D{i:3;uuid:"fb09fa68-07b0-5e3d-ab6a-51d549302bbe"}
}
##^##*/

