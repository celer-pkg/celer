// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    ZoomBlur {
        anchors.fill: parent
        source: bug
        length: 48.0
        samples: 24
        horizontalOffset: 3
        verticalOffset: -3
    }
}
