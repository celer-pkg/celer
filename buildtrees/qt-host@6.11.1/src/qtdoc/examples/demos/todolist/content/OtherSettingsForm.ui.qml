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

Page {
    id: root

    property alias otherSettingsModel: otherSettings.model
    property alias titleText: navBar.titleText
    property alias backButton: navBar.backButton

    header: NavBar {
        id: navBar

        previousPageTitle: qsTr("Settings")
        acceptButton.visible: false
    }

    ListView {
        id: otherSettings

        width: parent.width
        height: contentHeight

        delegate: SwitchDelegate {
            id: option

            required property var modelData

            width: parent.width
            text: modelData.name
            checked: modelData.checked

            Connections {
                function onClicked() {
                    option.modelData.actionOnClicked()
                }
            }
        }
    }
}
