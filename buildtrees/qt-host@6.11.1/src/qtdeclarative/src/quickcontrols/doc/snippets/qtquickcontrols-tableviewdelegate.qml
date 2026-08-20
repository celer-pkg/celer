// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

Window {
    width: 460
    height: 198
    visible: true
    title: qsTr("TableViewDelegate Example")

    TableView {
        id: tableView
        anchors.fill: parent
        model: tableModel
        delegate: tableDelegate
        selectionModel: ItemSelectionModel {}
    }

    Component {
        id: tableDelegate

        TableViewDelegate {
            topPadding: 8
            leftPadding: 12
            rightPadding: leftPadding
            bottomPadding: topPadding
            Component.onCompleted: contentItem.verticalAlignment = Text.AlignVCenter
        }
    }

    TableModel {
        id: tableModel

        TableModelColumn { display: "name"; edit: "name" }
        TableModelColumn { display: "address"; edit: "address" }
        TableModelColumn { display: "quantity"; edit: "quantity" }

        rows: [
            {
                name: "Kristian Quan",
                address: "123 Company Place, Big City",
                quantity: 4,
            },
            {
                name: "Matthew Rand",
                address: "The Orchard, Little Village",
                quantity: 2,
            },
            {
                name: "Eirik Asaki",
                address: "497 Park Skyway, Future City",
                quantity: 29,
            },
            {
                name: "Jarek Hanssen",
                address: "1023 RivieraDrive, Southern Precinct",
                quantity: 45,
            },
            {
                name: "Charlos Hartmann",
                address: "The Manor House, Country Estate",
                quantity: 1,
            },
            {
                name: "Bea King",
                address: "Floor 201, Sun Tower, Central City",
                quantity: 32,
            },
        ]
    }
}
