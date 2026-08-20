// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Window
import QtQuick.Controls

Rectangle {
    visible: true
    width: 800
    height: 800

    Column {
        Accessible.id: "Header1"
        Accessible.role: Accessible.Form
        Accessible.description: "Header 1"
        Row {
            Accessible.id: "Header2"
            Accessible.role: Accessible.Form
            Accessible.description: "Header 2"
            TextInput {
                Accessible.id: "text1"
                Accessible.description: "description is This is the TextInput area"
                text: "text is TextInput"
            }
            TextArea {
                Accessible.id: "text2"
                Accessible.description: "description is This is the TextArea area"
                text: "text is TextArea"
            }
            Text {
                Accessible.id: "text3"
                Accessible.description: "description is This is the Text area"
                Accessible.role: Accessible.EditableText
                text: "text is Text"
            }
            TextField {
                Accessible.id: "text4"
                Accessible.description: "description is This is the TextField area"
                Accessible.role: Accessible.EditableText
                text: "text is TextField"
            }
        }
        Row {
            TextInput {
                Accessible.id: "texts1"
                Accessible.description: "description is This is the static TextInput area"
                Accessible.role: Accessible.StaticText
                Accessible.name: text
                text: "text is static TextInput"
            }
            TextArea {
                Accessible.id: "texts2"
                Accessible.description: "description is This is the static TextArea area"
                Accessible.role: Accessible.StaticText
                Accessible.name: text
                text: "text is static TextArea"
            }
            Text {
                Accessible.id: "texts3"
                Accessible.description: "description is This is the static Text area"
                Accessible.role: Accessible.StaticText
                Accessible.name: text
                text: "text is static Text"
            }
            TextField {
                Accessible.id: "texts4"
                Accessible.description: "description is This is the static TextField area"
                Accessible.role: Accessible.StaticText
                Accessible.name: text
                text: "text is static TextField"
            }
        }
        Row {
            TextInput {
                Accessible.id: "texth1"
                Accessible.description: "description is This is the hidden TextInput area"
                Accessible.role: Accessible.EditableText
                text: "text is hidden TextInput"
                echoMode: TextInput.Password
            }
            TextField {
                Accessible.id: "texth4"
                Accessible.description: "description is This is the hidden Text field"
                Accessible.role: Accessible.EditableText

                text: "text is hidden TextField"
                echoMode: TextInput.Password
            }
        }
        Row {
            TextInput {
                Accessible.id: "textne1"
                Accessible.description: "description is This is the hidden TextInput area"
                Accessible.role: Accessible.EditableText
                text: "text is hidden TextInput"
                echoMode: TextInput.NoEcho
            }
            TextField {
                Accessible.id: "textne4"
                Accessible.description: "description is This is the hidden Text field"
                Accessible.role: Accessible.EditableText

                text: "text is hidden TextField"
                echoMode: TextInput.NoEcho
            }
        }
        Row {
            TextInput {
                Accessible.id: "textpe1"
                Accessible.description: "description is This is the hidden TextInput area"
                Accessible.role: Accessible.EditableText
                text: "text is hidden TextInput"
                echoMode: TextInput.PasswordEchoOnEdit
            }
            TextField {
                Accessible.id: "textpe4"
                Accessible.description: "description is This is the hidden Text field"
                Accessible.role: Accessible.EditableText

                text: "text is hidden TextField"
                echoMode: TextInput.PasswordEchoOnEdit
            }
        }
    }
}
