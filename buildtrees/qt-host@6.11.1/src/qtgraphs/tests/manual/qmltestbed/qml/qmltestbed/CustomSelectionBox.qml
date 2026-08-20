// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: comboBox
    property string text
    property string selection
    property alias values: combo.model
    property alias index: combo.currentIndex
    property real parentWidth

    width: rowLayout.width
    height: rowLayout.height

    RowLayout {
        id: rowLayout
        ComboBox {
            id: combo
            displayText: comboBox.text + ": " + model[currentIndex]
            textRole: comboBox.text
            implicitWidth: Math.max(comboBox.parentWidth - 5, 100)

            delegate: ItemDelegate {
                id: lightDelegate
                required property string modelData
                text: modelData
                anchors.left: parent.left
                anchors.right: parent.right
            }
            onCurrentIndexChanged: {
                comboBox.selection = model[currentIndex]
            }
        }
    }
}
