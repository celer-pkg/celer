// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Window
import QtQuick.Controls

Rectangle {
    visible: true
    width: 640
    height: 600

    Row {

    Column {
        Accessible.role: Accessible.Form
        Accessible.description: "Header 1"
        Row {
            Accessible.role: Accessible.Form
            Accessible.description: "Header 2A"
            CheckBox {
                Accessible.id: "checkboxa1"
                Accessible.description: "Button A1"
                text: "ButtonA1"
            }
            CheckBox {
                Accessible.id: "checkboxa2"
                Accessible.description: "Button A2"
                text: "ButtonA2"
            }
        }
        Row {
            Accessible.role: Accessible.Form
            Accessible.description: "Header 2B"
            RadioButton {
                Accessible.id: "radiobuttonb1"
                Accessible.description: "Button B1"
                text: "ButtonB1"
            }
            RadioButton {
                Accessible.id: "radiobuttonb2"
                Accessible.description: "Button B2"
                text: "ButtonB2"
            }
        }
    }
    Column {
        Row {
            Accessible.role: Accessible.Form
            Accessible.description: "Header 2C"
            Button {
                Accessible.id: "pushbuttonc1"
                Accessible.description: "Button C1"
                text: "ButtonC1"
                onClicked: {
                    text = "ButtonC1 - clicked"
                }
            }
            Button {
                Accessible.id: "pushbuttonc2"
                Accessible.description: "Button C2"
                text: "ButtonC2"
                onClicked: {
                    text = "ButtonC2 - clicked"
                }
            }
        }
    }
    }
}
