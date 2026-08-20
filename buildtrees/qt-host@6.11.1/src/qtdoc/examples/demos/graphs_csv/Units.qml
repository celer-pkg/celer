// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQuick

Item {
    property var window

    readonly property real px: {
        if (window === undefined)
            return 1;
        const win = window;
        return Math.max(win.width, win.height) / win.width;
    }
}
