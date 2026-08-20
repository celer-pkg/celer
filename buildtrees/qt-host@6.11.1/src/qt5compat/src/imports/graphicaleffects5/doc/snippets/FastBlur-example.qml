// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [example]
import QtQuick
//! [import]
import Qt5Compat.GraphicalEffects
//! [import]

Item {
    width: 300
    height: 300

    Image {
        id: bug
        source: "images/bug.jpg"
        sourceSize: Qt.size(parent.width, parent.height)
        smooth: true
        visible: false
    }

    FastBlur {
        anchors.fill: bug
        source: bug
        radius: 32
    }
}
//! [example]
