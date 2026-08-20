// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property color textColor: "black"
    property string textLabel: "TEXT"
    Text {
        text: textLabel
        font.pixelSize: 40
        color: textColor
    }
}
