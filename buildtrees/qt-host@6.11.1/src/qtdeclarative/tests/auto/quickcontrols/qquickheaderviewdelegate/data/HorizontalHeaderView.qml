// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    width: 300
    height: 300

    readonly property alias headerView: headerView

    HorizontalHeaderView {
        id: headerView
        x: 50
        width: 250
        height: 50

        model: ["Column 1", "Column 2", "Column 3", "Column 4"]
    }
}
