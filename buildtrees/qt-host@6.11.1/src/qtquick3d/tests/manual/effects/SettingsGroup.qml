// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Column {
    id: group
    width: parent.width
    property bool frame: true
    leftPadding: 10
    rightPadding: 5
    topPadding: 5
    bottomPadding: 5
    Item {
        visible: frame
        Rectangle {
            width: group.width
            height: group.height
            color: "#f1f1f0"
            border.color: "darkgray"
            border.width: 2
        }
    }
}
