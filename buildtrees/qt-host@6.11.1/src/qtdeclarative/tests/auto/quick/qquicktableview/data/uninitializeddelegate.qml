// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    width: 300
    height: 300

    property alias tableView: tableView
    property alias selectionRectangle: selectionRectangle

    TableView {
        id: tableView
        anchors.fill: parent
        interactive: false
        selectionModel: itemSelectionModel
    }

    ItemSelectionModel {
        id: itemSelectionModel
    }

    SelectionRectangle {
        id: selectionRectangle
        target: tableView
    }
}
