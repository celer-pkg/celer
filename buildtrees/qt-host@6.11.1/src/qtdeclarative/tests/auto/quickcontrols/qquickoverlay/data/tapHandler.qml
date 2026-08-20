// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 400
    height: 400

    Drawer {
        width: 100
        height: parent.height
        // Doesn't need to be visible; as long as it exists it will cause Overlay
        // to pay attention to mouse events so that it can emit pressed & released.
    }

    TapHandler {
        parent: window.Overlay.overlay
    }
}
