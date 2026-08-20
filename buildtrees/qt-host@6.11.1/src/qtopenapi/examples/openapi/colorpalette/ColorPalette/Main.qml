// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Color Palette")

    ServerSelection {
        id: serverview
        anchors.fill: parent
        onServerSelected: {colorview.visible = true; serverview.visible = false}
    }

    ColorView {
        id: colorview
        anchors.fill: parent
        visible: false // initially invisible
        onErrorOccurred: {colorview.visible = false; serverview.visible = true}
    }
}
