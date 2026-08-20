// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0

Rectangle {
    id: rootItem

    property color borderColor
    property real borderWidth

    width: 100
    height: 100
    radius: width / 2
    border.color: rootItem.borderColor
    border.width: rootItem.borderWidth
}
