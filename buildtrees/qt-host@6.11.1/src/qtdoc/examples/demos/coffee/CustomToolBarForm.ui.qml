// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic

Item {
    // Height, width and any other size related properties containing odd looking float or other dividers
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    id: root
    property alias backButton: backButton
    property alias themeButton: themeButton
    property alias logo: logo

    RowLayout {
        width: parent.width
        Button {
            id: backButton
            icon.color: (Colors.currentTheme == Colors.dark) ? "#FFFFFF" : "#121111"
            icon.source: "./images/icons/keyboard_backspace_black_left.svg"
            flat: true
            background: null
            transitions: Transition {
                NumberAnimation {
                    properties: "scale"
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Image {
            id: logo
            source: "./images/icons/Qt-logo-white-transparent.svg"
            Layout.alignment: Qt.AlignCenter
            sourceSize.width: height * 1.4
            sourceSize.height: 25
        }
        Button {
            id: themeButton
            Layout.alignment: Qt.AlignRight
            icon.color: (Colors.currentTheme == Colors.dark) ? "#DDE2E8" : "#667085"
            icon.source: (Colors.currentTheme == Colors.dark) ? "./images/icons/light_mode_black_24dp.svg" : "./images/icons/dark_mode_black_24dp.svg"
            flat: true
            background: null
            transitions: Transition {
                NumberAnimation {
                    properties: "scale"
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
