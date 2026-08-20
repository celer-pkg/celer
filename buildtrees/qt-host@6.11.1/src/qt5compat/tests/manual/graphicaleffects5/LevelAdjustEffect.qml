// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    LevelAdjust {
        anchors.fill: parent
        source: butterfly
        minimumOutput: "#00ffffff"
        maximumOutput: "#ff000000"
    }
}
