// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: checkbox
    width: 580
    height: 88
    property alias item_nameText: item_name.text
    state: "state_state_Idle"

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 580
        height: 88
        color: "#222222"
        radius: 8
    }

    CheckboxImages_1 {
        id: checkboxImages
        x: 8
        y: 8
        width: 72
        height: 72
        state: "state_type_Materials_Number_1"
    }

    Text {
        id: item_name
        x: 108
        y: 30
        width: 389
        height: 28
        color: "#ffffff"
        text: qsTr("Item name")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        font.family: "Archivo"
        font.weight: Font.Normal
    }

    IconsCheckbox_1 {
        id: iconsCheckbox
        x: 524
        y: 30
        width: 28
        height: 28
        clip: true
        state: "state_name_CheckEmpty"
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

            PropertyChanges {
                target: iconsCheckbox
                state: "state_name_CheckFilled"
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;uuid:"97d01e06-eea5-5b77-8d98-8551fdcb3a24"}D{i:1;uuid:"acbb415b-f966-5d42-9c0d-44b939fb8acb"}
D{i:2;uuid:"6782aee3-77ae-53ec-abf2-e1a5ea758261"}D{i:3;uuid:"07ed43fe-f485-5015-a68c-4b8dcc05e565"}
D{i:4;uuid:"6aff6e75-eba9-5616-8028-1339982141e4"}
}
##^##*/

