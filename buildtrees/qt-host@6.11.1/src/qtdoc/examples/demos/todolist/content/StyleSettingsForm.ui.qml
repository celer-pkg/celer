// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import ToDoList

Page {
    id: root

    property alias builtInStyles: stylesList.model
    property alias infoLabel: infoLabel
    property alias backButton: navBar.backButton
    property alias styleSettingsButton: group

    header: NavBar {
        id: navBar

        titleText: qsTr("Style")
        previousPageTitle: qsTr("Settings")
        acceptButton.visible: false
    }

    ToolTip {
        id: infoLabel

        y: parent.height - infoLabel.implicitHeight

        timeout: 8000
        text: qsTr("Please restart the application to apply changes")
    }

    ButtonGroup {
        id: group
    }

    ListView {
        id: stylesList

        width: parent.width
        height: contentHeight

        delegate: RadioDelegate {
            required property string modelData

            width: parent.width
            text: modelData
            checked: text === AppSettings.style

            ButtonGroup.group: group
        }
    }
}
