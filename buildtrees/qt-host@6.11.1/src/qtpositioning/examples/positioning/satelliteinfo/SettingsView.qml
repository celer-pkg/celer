// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    id: root

    signal showHelp
    signal toggleMode

    color: Theme.backgroundColor

    component Entry: ItemDelegate {
        id: entryRoot

        property bool heading: false
        property alias source: entryRoot.icon.source
        readonly property int iconSize: entryRoot.font.pixelSize

        icon.height: iconSize
        icon.width: iconSize
        icon.color: Theme.iconNormal
        palette.text: Theme.iconTextNormal
        display: AbstractButton.TextBesideIcon
        font.pixelSize: entryRoot.heading ? Theme.largeFontSize : Theme.mediumFontSize
        font.weight: Theme.fontDefaultWeight

        LayoutMirroring.enabled: true
        LayoutMirroring.childrenInherit: true

        background: Rectangle {
            anchors.fill: parent
            color: entryRoot.heading ? "transparent" : Theme.settingsEntryBackground
            Rectangle {
                height: 1
                width: parent.width
                anchors.bottom: parent.bottom
                color: Theme.settingsSeparatorColor
            }
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: Theme.defaultSpacing
        }
        spacing: 0

        Entry {
            heading: true
            text: qsTr("Settings")
            bottomPadding: 20
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
        }

        Entry {
            text: qsTr("Light Mode")
            source: "icons/lightmode.svg"
            visible: Theme.darkMode
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            onClicked: root.toggleMode()
        }

        Entry {
            text: qsTr("Dark Mode")
            source: "icons/darkmode.svg"
            visible: !Theme.darkMode
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            onClicked: root.toggleMode()
        }

        Entry {
            text: qsTr("Help")
            source: "icons/help.svg"
            Layout.alignment: Qt.AlignRight
            Layout.fillWidth: true
            onClicked: root.showHelp()
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
