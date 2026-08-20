// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick.Controls

//! [root]
Pane {
    anchors.fill: parent

    ContextMenu.menu: Menu {
        MenuItem {
            text: qsTr("Eat Tomato")
            onTriggered: { /* ... */ }
        }
        MenuItem {
            text: qsTr("Throw Tomato")
            onTriggered: { /* ... */ }
        }
        MenuItem {
            text: qsTr("Squash Tomato")
            onTriggered: { /* ... */ }
        }
    }
}
//! [root]
