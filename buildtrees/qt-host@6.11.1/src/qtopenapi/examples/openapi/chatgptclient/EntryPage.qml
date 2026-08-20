// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import chat_example

pragma ComponentBehavior: Bound

Item {
    id: root
    anchors.fill: parent

    signal choiceIsDone()

    ComboBox {
        id: availableModels
        anchors.bottom: charachters.top
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        width: charachters.width
        height: 40

        font.family: "Segoe UI"
        font.pointSize: 12

        contentItem: Text {
            text: availableModels.displayText
            font: availableModels.font
            color: "#FFFFFF"
            verticalAlignment: Text.AlignVCenter
            leftPadding: 10
        }

        delegate: ItemDelegate {
            id: comboItem
            width: ListView.view.width

            required property string modelData

            contentItem: Text {
                text: comboItem.modelData
                font.family: "Segoe UI"
                font.pointSize: 12
                color: "#FFFFFF"
                verticalAlignment: Text.AlignVCenter
            }
        }

        model: OpenAIChatManager.modelList
        displayText: count > 0 ? currentText : "No models available"

        onCurrentTextChanged: {
            OpenAIChatManager.modelName = currentText
        }
    }

    ListModel {
        id: choices
        ListElement {
            elementId: OpenAIChatManager.Wizard
            elementName: "Wizard"
            elementImage: "magic-hat.svg"
            elementColor1: "#2A2440"
            elementColor2: "#4B2E83"
            elementColor3: "#7A5CFF"
            pressedElementColor1: "#211C33"
            pressedElementColor2: "#3E256B"
            pressedElementColor3: "#654BE0"
        }
        ListElement {
            elementId: OpenAIChatManager.Scientist
            elementName: "Scientist"
            elementImage: "graduation-hat.svg"
            elementColor1: "#24303A"
            elementColor2: "#2C6E9B"
            elementColor3: "#4CC9F0"
            pressedElementColor1: "#1C252D"
            pressedElementColor2: "#255C82"
            pressedElementColor3: "#3FB0D6"
        }
        ListElement {
            elementId: OpenAIChatManager.DefaultMode
            elementName: "Regular mode"
            elementImage: "head.svg"
            elementColor1: "#2E2E2E"
            elementColor2: "#3A3A3A"
            elementColor3: "#505050"
            pressedElementColor1: "#242424"
            pressedElementColor2: "#303030"
            pressedElementColor3: "#444444"
        }
        ListElement {
            elementId: OpenAIChatManager.UserCharacter
            elementName: "Your choice"
            elementImage: "head-plus.svg"
            elementColor1: "#2C2C35"
            elementColor2: "#6A4DFF"
            elementColor3: "#5CC8FF"
            pressedElementColor1: "#23232A"
            pressedElementColor2: "#563ED6"
            pressedElementColor3: "#49AEE0"
        }
    }

    Grid {
        id: charachters

        anchors.centerIn: parent
        columns: 2
        spacing: 10
        Repeater {
            model: choices
            delegate: Rectangle {
                id: itemChoice
                width: 180
                height: 180

                required property int elementId
                required property string elementName
                required property string elementImage
                required property color elementColor1
                required property color elementColor2
                required property color elementColor3
                required property color pressedElementColor1
                required property color pressedElementColor2
                required property color pressedElementColor3

                gradient: Gradient {
                    GradientStop { position: 0.0; color: mouseArea.isPressed
                                                         ? itemChoice.pressedElementColor1 : itemChoice.elementColor1 }
                    GradientStop { position: 0.5; color: mouseArea.isPressed
                                                         ? itemChoice.pressedElementColor2 : itemChoice.elementColor2 }
                    GradientStop { position: 1.0; color: mouseArea.isPressed
                                                         ? itemChoice.pressedElementColor3 : itemChoice.elementColor3 }
                }

                Image {
                    id: icon
                    anchors.centerIn: parent

                    width: 50
                    height: 50

                    source: "resources/" + itemChoice.elementImage
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                Text {
                    anchors.top: icon.bottom
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: itemChoice.elementName
                    font.pointSize: 12
                }

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    enabled: OpenAIChatManager.modelName !== ""
                    property bool isPressed: false

                    onPressed: {
                        isPressed = true
                    }
                    onReleased: {
                        isPressed = false
                    }
                    onClicked: {
                        OpenAIChatManager.characterId = itemChoice.elementId
                        root.choiceIsDone()
                    }
                }
            }
        }
    }
}
