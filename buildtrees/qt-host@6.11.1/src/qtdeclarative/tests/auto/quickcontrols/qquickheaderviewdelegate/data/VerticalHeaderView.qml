// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    width: 300
    height: 300

    readonly property alias headerView: headerView

    VerticalHeaderView {
        id: headerView
        y: 50
        width: 50
        height: 250

        model: 10
    }
}
