// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//! [root]
Button {
    id: fileButton
    text: "File"
    onClicked: menu.open()

    Menu {
        id: menu
        y: fileButton.height

        MenuItem {
            text: "New..."
        }
        MenuItem {
            text: "Open..."
        }
        MenuItem {
            text: "Save"
        }
    }
}
//! [root]
