// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Rectangle {
    id: layoutFull
    width: 1920
    height: 1080
    color: "#00ffffff"
    border.color: "#00000000"
    property bool debugOn: button.checked

    required property NumberAnimation openmenuTransition
    required property real openmenuTransitionPhase

    LabelQt {
        id: labelQt
        x: 1644
        y: 972
        width: 248
        height: 80

        Button {
            id: button
            opacity: 0
            text: qsTr("")
            anchors.fill: parent
            checkable: true
            display: AbstractButton.TextOnly
        }
    }

    Button {
        id: settingsbutton
        x: 28
        y: 972
        width: 311
        height: 80
        scale: 1.0 - layoutFull.openmenuTransitionPhase * 0.5
        opacity: 1 - layoutFull.openmenuTransitionPhase
        state: "state_state_Idle"

        background: Rectangle {
            implicitWidth: 0
            implicitHeight: 0
            opacity: 0
            border.color: "#000000"
            border.width: 0
            radius: 2
            color: settingsbutton.pressed ? "#B8B8B9" : (settingsbutton.hovered ? "#5C5C5D" : (settingsbutton.checked ? "#3D3D3E" : "#222222"))
        }

        Connections {
            target: settingsbutton
            function onClicked() {
                layoutFull.openmenuTransition.start()
            }
        }

        Rectangle {
            id: bg
            x: 0
            y: 0
            width: 311
            height: 80
            color: settingsbutton.pressed ? "#B8B8B9" : (settingsbutton.hovered ? "#5C5C5D" : (settingsbutton.checked ? "#3D3D3E" : "#222222"))
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
            font.weight: Font.Bold
            font.family: "Archivo"
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
    }
}

/*##^##
Designer {
    D{i:0;height:1080;width:1920}D{i:1;uuid:"462816af-d13a-5644-b82e-550e6ea71900"}D{i:5;uuid:"c7c4867f-637a-5518-aceb-7f1dd2290416"}
D{i:6;uuid:"5266f2b1-d65a-5433-a9f7-76f929ec337c"}D{i:7;uuid:"fb09fa68-07b0-5e3d-ab6a-51d549302bbe"}
}
##^##*/

