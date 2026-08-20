// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: state_Idle
    width: 580
    height: 88
    property string checkboxImagesState: "state_type_Materials_Number_1"
    property alias item_nameText: item_name.text

    required property ListSelection selection

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 580
        height: 88
        color: button.pressed ? "#B8B8B9" : (button.hovered ? "#5C5C5D" : (button.checked ? "#3D3D3E" : "#222222"))
        radius: 8
    }

    CheckboxImages_1 {
        id: checkboxImages
        x: 8
        y: 8
        width: 72
        height: 72
        state: state_Idle.checkboxImagesState
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
        layer.smooth: false
        layer.format: ShaderEffectSource.RGBA
        layer.enabled: true
        renderTypeQuality: Text.VeryHighRenderTypeQuality
        enabled: true
        smooth: false
        antialiasing: true
        font.hintingPreference: Font.PreferDefaultHinting
        renderType: Text.NativeRendering
        font.family: "Archivo"
        font.weight: Font.Normal
    }

    IconsCheckbox_1 {
        id: iconsCheckbox
        x: 524
        y: 30
        width: 28
        height: 28
        state: state_Idle.selection.selection === state_Idle.item_nameText ? "state_name_CheckFilled"
                                                           : "state_name_CheckEmpty"
    }

    Button {
        id: button
        opacity: 0
        text: qsTr("")
        anchors.fill: parent
        checkable: true
        checked: state_Idle.selection.selection === state_Idle.item_nameText
        Connections {
            target: button
            function onClicked() {
                state_Idle.selection.selection = state_Idle.item_nameText
            }
        }
    }

    Item {
        id: __materialLibrary__
    }
}

/*##^##
Designer {
    D{i:0;height:88;width:580}D{i:1;uuid:"0d8ea12a-c87c-56f2-a989-694e642387f9"}D{i:2;uuid:"6e1c9cdb-ebb3-59f8-875a-56612e28949e"}
D{i:3;uuid:"aef0d473-f1a3-5cdc-a25c-6a24bda97c05"}D{i:4;uuid:"766d8034-0b03-5b52-b443-99b705d3fa2a"}
}
##^##*/

