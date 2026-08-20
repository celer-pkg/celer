// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    property int constant: 640
    property int height: 480
    property int t: constant + height
                    + constant++
                    + height++
                    + constant--
                    + height--
                    + --constant
                    + --height
                    + ++constant
                    + ++height
}
