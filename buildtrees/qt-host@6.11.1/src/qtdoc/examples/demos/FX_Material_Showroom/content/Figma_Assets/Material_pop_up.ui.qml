// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: material_pop_up

    required property real openmenuTransitionPhase
    required property NumberAnimation closemenuTransition
    required property QtObject screen

    x: (1 - openmenuTransitionPhase) * -636
    width: 636
    height: 1080
    opacity: openmenuTransitionPhase
    property alias setting_panelText: setting_panel.text
    property alias effectsL: listEffects
    property alias modelsL: listModels
    property alias matsL: listMaterial
    property alias matSelection: listMaterial.selection
    state: "state_type_Materials"

    Rectangle {
        id: bg
        anchors.fill: parent
        color: "#222222"
    }

    Scrollbar {
        id: scrollbar
        y: 288
        width: 4
        height: 764
        anchors.right: parent.right
        anchors.rightMargin: 12
        scrollPos: mouseScroll.normalizedValue > 1 ? 1 : mouseScroll.normalizedValue
    }

    MouseScroll {
        id: mouseScroll
        width: bg.width
        height: bg.height
        anchors.top: parent.top
        anchors.topMargin: 300
        maxValue: 200
        minValue: 0
    }

    MouseArea {
        id: mouseAreaList
        width: bg.width
        height: bg.height

        Connections {
            target: mouseAreaList
            property real speed: 0.3
            function onWheel(wheel) {
                mouseScroll.curValue = (mouseScroll.curValue + wheel.angleDelta.y * speed < mouseScroll.minValue)
                        ? mouseScroll.minValue
                        : ((mouseScroll.curValue + wheel.angleDelta.y * speed > mouseScroll.maxValue)
                           ? mouseScroll.maxValue
                           : mouseScroll.curValue + wheel.angleDelta.y * speed)
            }
        }
    }

    ListMaterial {
        id: listMaterial
        x: 28
        y: 288
        width: 580
        height: 636
        visible: false
    }

    ListEffects {
        id: listEffects
        x: 28
        y: 288
        width: 580
        height: 1050
        visible: false
    }

    ListModels {
        id: listModels
        x: 28
        y: 288
        width: 580
        height: 376
        visible: false
    }

    Rectangle {
        id: bg1
        height: 288
        color: "#222222"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
    }

    IconsFunctional {
        id: iconsFunctional
        width: 48
        height: 48
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 28
        clip: true
        state: "state_icon_Close"
        anchors.verticalCenterOffset: -488

        Button {
            id: button
            opacity: 0
            text: qsTr("")
            anchors.fill: parent
            display: AbstractButton.TextOnly

            Connections {
                target: button
                function onClicked() {
                    material_pop_up.screen.settingsopen = false
                }
            }

            Connections {
                target: button
                function onClicked() {
                    material_pop_up.closemenuTransition.start()
                }
            }
        }
    }

    Text {
        id: setting_panel
        x: 28
        y: 32
        width: 176
        height: 40
        color: "#ffffff"
        text: qsTr("Setting panel")
        font.pixelSize: 30
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        font.family: "Archivo"
        font.weight: Font.Normal
    }

    CardMaterial {
        id: card3DModels
        popUp: material_pop_up
        x: 420
        y: 100
        width: 188
        height: 160
        materialText: "3D models"
        selectionID: "state_type_3D_models"
        iconsCardsState: "state_name_3D_model"
    }

    CardMaterial {
        id: cardMaterial
        popUp: material_pop_up
        x: 28
        y: 100
        width: 188
        height: 160
        materialText: "Materials"
        selectionID: "state_type_Materials"
        iconsCardsState: "state_name_Material"
    }

    CardMaterial {
        id: cardEffects
        popUp: material_pop_up
        x: 224
        y: 100
        width: 188
        height: 160
        materialText: "Effects"
        selectionID: "state_type_Effects"
        iconsCardsState: "state_name_Effects"
    }

    states: [
        State {
            name: "state_type_Materials"

            PropertyChanges {
                target: listMaterial
                visible: true
                y: 288 - (mouseScroll.curValue > 180 ? 180 : mouseScroll.curValue)
            }

            PropertyChanges {
                target: cardMaterial
                state: "state_state_Active"
            }

            PropertyChanges {
                target: mouseScroll
                maxValue: 180
            }
        },
        State {
            name: "state_type_Effects"

            PropertyChanges {
                target: listEffects
                visible: true
                y: 288 - mouseScroll.curValue
            }

            PropertyChanges {
                target: scrollbar
                visible: true
            }

            PropertyChanges {
                target: cardEffects
                state: "state_state_Active"
            }

            PropertyChanges {
                target: mouseScroll
                maxValue: 280
            }
        },
        State {
            name: "state_type_3D_models"

            PropertyChanges {
                target: listModels
                visible: true
            }

            PropertyChanges {
                target: scrollbar
                visible: false
            }

            PropertyChanges {
                target: card3DModels
                state: "state_state_Active"
            }
        }
    ]
}

/*##^##
Designer {
    D{i:0;height:1080;width:636}D{i:1;uuid:"86f97c70-70e8-521e-89d9-030a4ad25d96"}D{i:2;uuid:"61d97dfb-bcb1-5349-b95e-1af2f40e6a16"}
D{i:6;uuid:"6a7f3dee-c8de-5bc9-bf1e-4fe9a90f6532"}D{i:7;uuid:"0ea32f9e-8e4f-5682-a9bb-5ab31dadcec5"}
D{i:8;uuid:"f9d3179e-957d-5adc-9ad7-3499fde6fb21"}D{i:9;uuid:"86f97c70-70e8-521e-89d9-030a4ad25d96"}
D{i:10;uuid:"7f9fbcda-099b-5ba4-8537-831fb61a0a4c"}D{i:14;uuid:"ad97d5ce-1828-5bf5-a684-6a124c8cd64d"}
D{i:15;uuid:"192c6cf4-144a-527d-b03e-6a7a2635cbfa"}D{i:16;uuid:"9b328eb3-eafa-54a7-ba9f-cc8f646a3a8c"}
D{i:17;uuid:"571e854d-d72c-523b-8318-4a7896028de1"}
}
##^##*/

