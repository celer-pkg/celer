// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

pragma Singleton

import QtQuick
import QtGraphs

QtObject {
    property var theme: GraphsTheme {
        theme: GraphsTheme.Theme.MixSeries
        labelFont.pointSize: 60
        plotAreaBackgroundVisible: false
        labelsVisible: false
        gridVisible: false
        colorScheme: GraphsTheme.ColorScheme.Dark
        labelTextColor: "red"
    }
}
