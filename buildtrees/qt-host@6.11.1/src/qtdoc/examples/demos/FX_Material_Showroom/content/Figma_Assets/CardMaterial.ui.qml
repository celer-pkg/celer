// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: cardMaterial
    width: 188
    height: 160
    property string iconsCardsState: "state_name_Materials"
    property alias materialText: material.text
    property string selectionID: "test"
    state: "state_state_Idle"

    required property Item popUp

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 188
        height: 160
        color: button.pressed ? "#B8B8B9" : (button.hovered ? "#5C5C5D" : (button.checked ? "#3D3D3E" : "#222222"))
        radius: 8
    }

    Text {
        id: material
        x: 20
        width: 149
        height: 28
        color: "#ffffff"
        text: qsTr("Material")
        anchors.bottom: parent.bottom
        font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        anchors.bottomMargin: 20
        font.family: "Archivo"
        font.weight: Font.Normal
    }

    IconsCards {
        id: iconsCards
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 58
        anchors.bottomMargin: 68
        clip: true
        anchors.topMargin: 20
        anchors.leftMargin: 58
        state: parent.iconsCardsState
    }

    Button {
        id: button
        opacity: 0
        text: qsTr("")
        anchors.fill: parent
        highlighted: true
        checkable: true
        checked: cardMaterial.popUp.state === cardMaterial.selectionID
        display: AbstractButton.TextOnly

        Connections {
            target: button
            function onClicked() {
                cardMaterial.popUp.state = cardMaterial.selectionID
            }
        }
    }
    states: [
        State {
            name: "state_state_Active"
        },
        State {
            name: "state_state_Pressed"
        },
        State {
            name: "state_state_Hover"
        },
        State {
            name: "state_state_Idle"

            PropertyChanges {
                target: iconsCards
                x: 58
                y: 20
                width: 72
                height: 72
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:160;width:188}D{i:1;uuid:"ea2df35d-3c75-51e1-8a6b-534bf039dd51"}D{i:2;uuid:"8e896bc3-37b4-5166-ab69-5d94de048ede"}
D{i:3;uuid:"630f982b-ec7f-51b6-9634-cb32f2584cf5"}
}
##^##*/

