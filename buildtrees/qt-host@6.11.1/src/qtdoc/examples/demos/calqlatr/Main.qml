// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Window {
    visible: true
    width: 320
    height: 480
    minimumWidth: Math.max(numberPad.portraitModeWidth, calcDisplay.minWidth) + root.margin * 2
    minimumHeight: calcDisplay.minHeight + numberPad.height + root.margin * 3
    color: root.backgroundColor

    Item {
        id: root
        anchors.fill: parent

        anchors.topMargin: parent.SafeArea.margins.top
        anchors.leftMargin: parent.SafeArea.margins.left
        anchors.rightMargin: parent.SafeArea.margins.right
        anchors.bottomMargin: parent.SafeArea.margins.bottom

        readonly property int margin: 18
        readonly property color backgroundColor: "#222222"
        readonly property int minLandscapeModeWidth: numberPad.landscapeModeWidth
                                                     + calcDisplay.minWidth + margin * 3

        property bool isPortraitMode: root.width < root.minLandscapeModeWidth

        ApplicationState {
            id: appState
            display: calcDisplay
        }

        Display {
            id: calcDisplay
            readonly property int minWidth: 210
            readonly property int minHeight: 60

            Layout.minimumWidth: minWidth
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: root.margin

            // remove the margin on the side that the numberPad is on, to prevent a double margin
            Layout.bottomMargin: root.isPortraitMode ? 0 : root.margin
            Layout.rightMargin: root.isPortraitMode ? root.margin : 0
        }

        NumberPad {
            id: numberPad
            Layout.margins: root.margin

            isPortraitMode: root.isPortraitMode
            applicationState: appState
        }

        // define the responsive layouts
        ColumnLayout {
            id: portraitMode
            anchors.fill: parent
            visible: root.isPortraitMode

            LayoutItemProxy {
                target: calcDisplay
                Layout.minimumHeight: calcDisplay.minHeight
            }
            LayoutItemProxy {
                target: numberPad
                Layout.alignment: Qt.AlignHCenter
            }
        }

        RowLayout {
            id: landscapeMode
            anchors.fill: parent
            visible: !root.isPortraitMode

            LayoutItemProxy {
                target: calcDisplay
            }
            LayoutItemProxy {
                target: numberPad
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Keys.onPressed: function (event) {
            switch (event.key) {
                case Qt.Key_0: appState.digitPressed("0"); break;
                case Qt.Key_1: appState.digitPressed("1"); break;
                case Qt.Key_2: appState.digitPressed("2"); break;
                case Qt.Key_3: appState.digitPressed("3"); break;
                case Qt.Key_4: appState.digitPressed("4"); break;
                case Qt.Key_5: appState.digitPressed("5"); break;
                case Qt.Key_6: appState.digitPressed("6"); break;
                case Qt.Key_7: appState.digitPressed("7"); break;
                case Qt.Key_8: appState.digitPressed("8"); break;
                case Qt.Key_9: appState.digitPressed("9"); break;
                case Qt.Key_E: appState.digitPressed("e"); break;
                case Qt.Key_P: appState.digitPressed("π"); break;
                case Qt.Key_Plus: appState.operatorPressed("+"); break;
                case Qt.Key_Minus: appState.operatorPressed("-"); break;
                case Qt.Key_Asterisk: appState.operatorPressed("×"); break;
                case Qt.Key_Slash: appState.operatorPressed("÷"); break;
                case Qt.Key_Enter:
                case Qt.Key_Return: appState.operatorPressed("="); break;
                case Qt.Key_Comma:
                case Qt.Key_Period: appState.digitPressed("."); break;
                case Qt.Key_Backspace: appState.operatorPressed("bs"); break;
            }
        }
    }
}
