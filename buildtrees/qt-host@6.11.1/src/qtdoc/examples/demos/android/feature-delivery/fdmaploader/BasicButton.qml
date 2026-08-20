// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: basicButton
    property alias buttonText: basicButton.text

    text: "The Button"
    Layout.minimumHeight: 10
    Layout.preferredHeight: 50
    Layout.minimumWidth: 75
    Layout.preferredWidth: 130

    background: Rectangle {
        radius: 2
        border.width: 2
        border.color: "darkgray"
    }
}
