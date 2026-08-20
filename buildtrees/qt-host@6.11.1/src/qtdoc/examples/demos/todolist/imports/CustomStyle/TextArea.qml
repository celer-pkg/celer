// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.TextArea {
    id: textArea

    padding: 16
    font.pixelSize: 16
    color: Constants.secondaryColor

    background: Rectangle {
        color: Constants.mainColor
        border.color: Constants.secondaryColor
        opacity: textArea.hovered || textArea.text ? 1 : 0.4
        radius: 4
    }
}
