// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
//! [children]
    TextArea {
        text: qsTr("TextArea")

        // Disable the built-in context menu (since Qt 6.9).
        ContextMenu.menu: null

        TapHandler {
            acceptedButtons: Qt.RightButton
            onPressedChanged: {
                if (pressed === (Application.styleHints.contextMenuTrigger === Qt.ContextMenuTrigger.Press))
                    contextMenu.popup()
            }
        }
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Cut")
            // ...
        }
        MenuItem {
            text: qsTr("Copy")
            // ...
        }
        MenuItem {
            text: qsTr("Paste")
            // ...
        }
    }
//! [children]
}
