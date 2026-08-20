// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 600
    height: 400

    SafeArea.additionalMargins.bottom: 20

    Rectangle {
        anchors.fill: parent
        color: "red"
    }

    footer: ToolBar {
        visible: false

        Rectangle {
            anchors.fill: parent
            color: "yellow"
            implicitHeight: 50;
        }
    }
}
