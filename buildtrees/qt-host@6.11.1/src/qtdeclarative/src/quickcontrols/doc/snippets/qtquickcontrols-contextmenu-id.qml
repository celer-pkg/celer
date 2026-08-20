// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick.Controls

//! [root]
Pane {
    anchors.fill: parent

    ContextMenu.menu: Menu {
        // This prevents lazy creation of the Menu.
        id: myMenu

        // ...
    }
}
//! [root]
