// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import TestModel
import QtQuick.Controls

Item {
    width: 800
    height: 600

    property alias tableView: tableView
    property alias selectionRectangle: selectionRectangle

    TableView {
        id: tableView
        anchors.fill:parent
        anchors.margins: 10
        model: TestModel {}
        clip: true

        delegate: TableViewDelegate {
            readonly property string displayText: contentItem ? contentItem.text : null
        }
        selectionModel: ItemSelectionModel { model: tableView.model }
    }

    SelectionRectangle {
        id: selectionRectangle
        target: tableView
    }
}
