// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: controller

    required property bool isPortraitMode
    required property ApplicationState applicationState

    readonly property color qtGreenColor: "#2CDE85"
    readonly property color backspaceRedColor: "#DE2C2C"
    readonly property int spacing: 5

    property int portraitModeWidth: mainGrid.width
    property int landscapeModeWidth: scientificGrid.width + mainGrid.width

    implicitWidth: isPortraitMode ? portraitModeWidth : landscapeModeWidth
    implicitHeight: mainGrid.height

    function updateDimmed() {
        for (let i = 0; i < mainGrid.children.length; i++) {
            mainGrid.children[i].dimmed = applicationState.isButtonDisabled(mainGrid.children[i].text);
        }
        for (let j = 0; j < scientificGrid.children.length; j++) {
            scientificGrid.children[j].dimmed = applicationState.isButtonDisabled(
                        scientificGrid.children[j].text);
        }
    }

    component DigitButton: CalculatorButton {
        onClicked: {
            controller.applicationState.digitPressed(text);
            controller.updateDimmed();
        }
    }

    component OperatorButton: CalculatorButton {
        dimmable: true
        implicitWidth: 48
        textColor: controller.qtGreenColor

        onClicked: {
            controller.applicationState.operatorPressed(text);
            controller.updateDimmed();
        }
    }

    Component.onCompleted: updateDimmed()

    Rectangle {
        id: numberPad
        anchors.fill: parent
        radius: 8
        color: "transparent"

        RowLayout {
            spacing: controller.spacing

            GridLayout {
                id: scientificGrid
                columns: 3
                columnSpacing: controller.spacing
                rowSpacing: controller.spacing
                visible: !controller.isPortraitMode

                OperatorButton {
                    text: "x²"
                    Accessible.name: "x squared"
                }
                OperatorButton {
                    text: "⅟x"
                    Accessible.name: "one over x"
                }
                OperatorButton { text: "√" }
                OperatorButton {
                    text: "x³"
                    Accessible.name: "x cubed"
                }
                OperatorButton {
                    text: "sin"
                    Accessible.name: "sine"
                }
                OperatorButton {
                    text: "|x|"
                    Accessible.name: "absolute value"
                }
                OperatorButton { text: "log" }
                OperatorButton {
                    text: "cos"
                    Accessible.name: "cosine"
                }
                DigitButton {
                    text: "e"
                    dimmable: true
                    implicitWidth: 48
                }
                OperatorButton { text: "ln" }
                OperatorButton { text: "tan" }
                DigitButton {
                    text: "π"
                    dimmable: true
                    implicitWidth: 48
                }
            }

            GridLayout {
                id: mainGrid
                columns: 5
                columnSpacing: controller.spacing
                rowSpacing: controller.spacing

                BackspaceButton {
                    onClicked: {
                        controller.applicationState.operatorPressed(this.text);
                        controller.updateDimmed();
                    }
                }

                DigitButton { text: "7" }
                DigitButton { text: "8" }
                DigitButton { text: "9" }
                OperatorButton {
                    text: "÷"
                    implicitWidth: 38
                }

                OperatorButton {
                    text: "AC"
                    textColor: controller.backspaceRedColor
                    accentColor: controller.backspaceRedColor
                }
                DigitButton { text: "4" }
                DigitButton { text: "5" }
                DigitButton { text: "6" }
                OperatorButton {
                    text: "×"
                    implicitWidth: 38
                }

                OperatorButton {
                    text: "="
                    implicitHeight: 81
                    Layout.rowSpan: 2
                }
                DigitButton { text: "1" }
                DigitButton { text: "2" }
                DigitButton { text: "3" }
                OperatorButton {
                    text: "−"
                    implicitWidth: 38
                }

                OperatorButton {
                    text: "±"
                    implicitWidth: 38
                }
                DigitButton { text: "0" }
                DigitButton {
                    text: "."
                    dimmable: true
                }
                OperatorButton {
                    text: "+"
                    implicitWidth: 38
                }
            }
        } // RowLayout
    }
}
