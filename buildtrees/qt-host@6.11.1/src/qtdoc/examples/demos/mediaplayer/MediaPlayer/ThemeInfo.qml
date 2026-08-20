// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import MediaControls
import Config

Item {
    id: root

    Item {
        anchors.fill: parent

        Column {
            padding: 15
            spacing: 20

            ButtonGroup { id: group }

            CustomRadioButton {
                checked: Config.Theme.Light === Config.activeTheme
                text: qsTr("Light theme")
                ButtonGroup.group: group
                onClicked: Config.activeTheme = Config.Theme.Light
            }

            CustomRadioButton {
                checked: Config.Theme.Dark === Config.activeTheme
                text: qsTr("Dark theme")
                ButtonGroup.group: group
                onClicked: Config.activeTheme = Config.Theme.Dark
            }
        }
    }
}
