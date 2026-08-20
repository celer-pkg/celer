// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Row {
    property real rating: 0
    property int size: 0

    Repeater {
        model: Math.ceil(rating)
        Image {
            source: Qt.resolvedUrl("../resources/star.png")
            width: size
            height: size
        }
    }
}
