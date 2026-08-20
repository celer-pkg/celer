// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Controls.Material
import ToDoList

Window {
    id: root

    property alias globalNavigationStackView: stackView

    width: 360
    height: 800
    minimumHeight: 600
    minimumWidth: 270
    visible: true

    Universal.theme: Constants.isDarkModeActive ? Universal.Dark : Universal.Light
    Material.theme: Constants.isDarkModeActive ? Material.Dark : Material.Light

    property var builtInStyles

    StackView {
        id: stackView

        anchors.fill: parent
        initialItem: HomePage {
            builtInStyles: root.builtInStyles
        }
    }
}
