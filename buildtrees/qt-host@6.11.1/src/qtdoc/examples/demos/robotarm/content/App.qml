// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import RobotArm

ApplicationWindow {
    width: Constants.width
    height: Constants.height

    minimumWidth: 800
    minimumHeight: 600

    visible: true
    title: "RobotArm"

    MainScreen {
        anchors.fill: parent
    }
}
