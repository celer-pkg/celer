// Copyright (C) 2025 The Qt Company Ltd.
// PDX-License-Identifier: BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts

// To complete the test, test every scenario described in the checkboxes with the TextFields.
// To consider the checkbox done, the software keyboard in use should perform up to standards
// with the current checkbox scenario.


ApplicationWindow {
    visible: true

    GridLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        columns: Screen.height > Screen.width ? 2 : 4
        columnSpacing: 10
        rowSpacing: 20

        Text {
            text: "Qt.ImhSensitiveData"
        }


        // Keyboard should not show word completion suggestions.
        TextField{
            inputMethodHints: Qt.ImhSensitiveData
        }

        CheckBox {
            text: qsTr("InputMethodHints")
            Layout.columnSpan: 2
        }

        Text {
            text: "Qt.ImhDigitsOnly"
        }

        // Keyboard should display only digits.
        TextField{
            inputMethodHints: Qt.ImhDigitsOnly
        }

        CheckBox {
            text: qsTr("InputMethodHints")
            Layout.columnSpan: 2
        }

        Text {
            text: "Default behavior"
        }

        TextField{
        }

        GridLayout {
            columns: 3
            Layout.columnSpan: 2

            // To complete, have already installed Microsoft Swiftkey on target device and
            // switch to that keyboard before completing this check.
            // Swiftkey can be downloaded from Playstore.
            CheckBox {
                text: qsTr("SwiftKey")
            }

            // To complete, have already installed GBoard on target device
            // and switch to that keyboard before completing this check.
            // GBoard can be installed from Playstore.
            CheckBox {
                text: qsTr("GBoard")
            }

            // To complete, switch keyboard language to one of: Japanese, Chinese or Korean.
            // As these languages use different kind of word composition.
            CheckBox {
                id: cjk
                text: qsTr("CJK languages")
                visible: true
            }

            // To complete, orient target device to landscape mode.
            CheckBox {
                text: qsTr("Landscape")
            }

            // To complete, orient target device to Portrait mode.
            CheckBox {
                text: qsTr("Portrait")
            }

            // To complete, open this application in split screen mode with
            // some other application (the other app might be any of your choosing).
            // More information can be found from developer.android.com
            CheckBox {
                text: qsTr("Split-screen")
            }

            // To complete, open this application in a Multi-window mode
            // the application should appear as detached resizable window on your
            // target device display.
            // More information can be found from developer.android.com
            CheckBox {
                text: qsTr("Multi-window mode")
            }
        }
    }
}
