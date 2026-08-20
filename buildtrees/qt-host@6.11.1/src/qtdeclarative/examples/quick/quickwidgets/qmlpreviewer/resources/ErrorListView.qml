// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

import ErrorListModel

Rectangle {
    implicitHeight: 200
    color: palette.base

    ListView {
        id: listView
        anchors.fill: parent
        model: errorModel

        delegate: ItemDelegate {
            id: delegateItem
            implicitWidth: ListView.view.width
            highlighted: ListView.isCurrentItem

            required property int index
            required property var model

            contentItem: Label {
                id: errorText
                color: delegateItem.highlighted ? palette.highlightedText
                                                : palette.buttonText
                text: delegateItem.model.display
            }

            background: Rectangle {
                color: delegateItem.highlighted ? palette.highlight : palette.base
            }

            MouseArea {
                anchors.fill: parent
                onClicked: delegateItem.ListView.view.currentIndex = delegateItem.index
                onDoubleClicked: delegateItem.ListView.view.model.selectRow(delegateItem.index)
            }
        }
    }
}
