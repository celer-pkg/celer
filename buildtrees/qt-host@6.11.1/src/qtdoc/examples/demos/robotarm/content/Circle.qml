// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root

    required property bool isFocused

    color: "transparent"
    radius: width / 2
    border.width: 2
    border.color: root.isFocused ? root.palette.accent : root.palette.text
}
