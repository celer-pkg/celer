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
import QtQuick.Layouts
import ToDoList

Page {
    id: root

    property alias backButton: navBar.backButton

    padding: 12

    header: NavBar {
        id: navBar

        titleText: qsTr("Font Size")
        previousPageTitle: qsTr("Settings")
        acceptButton.visible: false
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: parent.topPadding

        spacing: 12

        Label {
            text: qsTr("A")
            font.pixelSize: 14
            font.weight: 400
            color: Constants.secondaryColor
        }

        Slider {
            id: slider

            Layout.fillWidth: true

            snapMode: Slider.SnapAlways
            stepSize: 1
            from: 14
            value: AppSettings.fontSize
            to: 21

            Connections {
                function onMoved() { AppSettings.fontSize = slider.value }
            }
        }

        Label {
            text: qsTr("A")
            font.pixelSize: 21
            font.weight: 400
            color: Constants.secondaryColor
        }
    }
}
