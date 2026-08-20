// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    //! [buttonAndMenu]
    Button {
        id: button
        text: qsTr("Click me!")
        ContextMenu.onRequested: position => {
            const menu = buttonMenu.createObject(button)
            menu.popup(position)
        }
    }

    Component {
        id: buttonMenu
        Menu {
            MenuItem { text: qsTr("Open") }
        }
    }
    //! [buttonAndMenu]
}
