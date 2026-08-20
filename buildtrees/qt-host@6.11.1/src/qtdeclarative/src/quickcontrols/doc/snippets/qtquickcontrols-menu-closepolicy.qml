// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

MouseArea {
//! [closePolicy]
    onClicked: menu.visible = !menu.visible

    Menu {
        id: menu
        // ...
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
//! [closePolicy]
    }
}
