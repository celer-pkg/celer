// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Window {
    flags: Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    visible: true

//![0]
Rectangle {
    id: parentItem
    gradient: Gradient.SunnyMorning
    anchors.fill: parent

    Rectangle {
        id: childItem
        gradient: Gradient.DustyGrass

        anchors {
            fill: parent

            topMargin: parent.SafeArea.margins.top
            leftMargin: parent.SafeArea.margins.left
            rightMargin: parent.SafeArea.margins.right
            bottomMargin: parent.SafeArea.margins.bottom
        }
    }
}
//![0]
}
