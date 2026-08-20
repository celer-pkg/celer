// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: word
    property string text: ""

    Row {
        id: row
        spacing: topLevel.width / 100
        anchors.fill: parent
        Repeater {
            id: letters
            model: text.length

            Letter {
                id: letter
                text: word.text.charAt(index)
                width: (word.width / word.text.length) - row.spacing
                height: word.height
            }
        }
    }
}
