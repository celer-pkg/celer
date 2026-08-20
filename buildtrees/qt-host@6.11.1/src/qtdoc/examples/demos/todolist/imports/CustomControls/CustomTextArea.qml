// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToDoList

Column {
    id: root

    property alias text: textArea.text

    spacing: 10

    RowLayout {
        id: labels

        width: parent.width

        Label {
            text: qsTr("Notes")
            font.pixelSize: AppSettings.fontSize - 2
            color: "#41CD52"
        }

        CustomLabel {
            id: finishEditing

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 8

            text: qsTr("Finish editing")
            visible: textArea.focus

            tapHandler.onTapped: textArea.focus = false
        }
    }

    ScrollView {
        width: root.width
        height: root.height - labels.height - spacing
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        TextArea {
            id: textArea

            wrapMode: TextArea.Wrap
        }
    }
}
