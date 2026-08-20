// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Window {
    flags: Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    visible: true

//![0]
Control {
    anchors.fill: parent

    background: Rectangle {
        gradient: Gradient.SunnyMorning
    }

    topPadding: SafeArea.margins.top
    leftPadding: SafeArea.margins.left
    rightPadding: SafeArea.margins.right
    bottomPadding: SafeArea.margins.bottom

    contentItem: Rectangle {
        gradient: Gradient.DustyGrass
    }
}
//![0]
}
