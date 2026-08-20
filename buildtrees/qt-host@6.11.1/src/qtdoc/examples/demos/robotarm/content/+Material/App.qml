// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Material
import RobotArm

ApplicationWindow {
    width: Constants.width
    height: Constants.height

    minimumWidth: 800
    minimumHeight: 600

    visible: true
    title: "RobotArm"

    Material.theme: mainScreen.darkMode ? Material.Dark : Material.Light

    MainScreen {
        id: mainScreen
        anchors.fill: parent
    }
}
