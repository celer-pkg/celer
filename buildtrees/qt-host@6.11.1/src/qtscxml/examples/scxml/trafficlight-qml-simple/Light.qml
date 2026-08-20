// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    property color color
    height: parent.height / 3
    width: height

    Rectangle {
        anchors.fill: parent
        anchors.margins: 3
        color: parent.color
        radius: width
    }
}

