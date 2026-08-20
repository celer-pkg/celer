// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import ToDoList

ToolBar {
    id: root

    property alias titleText: title.text
    property alias backButton: backButton
    property alias acceptButton: acceptButton
    property alias previousPageTitle: backButton.text
    property alias acceptButtonVisible: acceptButton.visible

    ToolButton {
        id: backButton
        anchors.left: parent.left
        anchors.leftMargin: 5
        icon.source: Constants.isDarkModeActive ? "images/LeftArrow_Icon.svg" : "images/LeftArrow_Icon_Dark.svg"
        text: qsTr("Tasks")

        palette.button: Constants.isDarkModeActive ? "#30D158" : "#34C759"
        palette.highlight: Constants.isDarkModeActive ? "#30DB5B" : "#248A3D"
    }

    Label {
        id: title
        text: qsTr("Settings")
        font.pixelSize: AppSettings.fontSize + 4
        horizontalAlignment: Text.AlignHCenter
        color: Constants.secondaryColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    ToolButton {
        id: acceptButton

        anchors.right: parent.right
        anchors.rightMargin: 5
        visible: false
        icon.source: "images/Check_Icon.svg"

        palette.button: Constants.isDarkModeActive ? "#30D158" : "#34C759"
        palette.highlight: Constants.isDarkModeActive ? "#30DB5B" : "#248A3D"
    }
}
