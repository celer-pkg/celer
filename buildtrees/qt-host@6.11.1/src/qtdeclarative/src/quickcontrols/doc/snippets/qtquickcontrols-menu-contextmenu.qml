// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
//! [children]
    TapHandler {
        acceptedButtons: Qt.RightButton
        onPressedChanged: {
            if (pressed && Application.styleHints.contextMenuTrigger === Qt.ContextMenuTrigger.Press)
                contextMenu.popup()
        }
        onTapped: {
            if (Application.styleHints.contextMenuTrigger === Qt.ContextMenuTrigger.Release)
                contextMenu.popup()
        }
    }
    TapHandler {
        acceptedDevices: PointerDevice.TouchScreen
        onLongPressed: contextMenu.popup()
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Do stuff")
        }
        MenuItem {
            text: qsTr("Do more stuff")
        }
    }
//! [children]
}
