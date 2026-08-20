// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

ApplicationWindow {
    width: 440
    height: 230
    visible: true
    title: qsTr("HeaderViewDelegate Example")

    HorizontalHeaderView {
        id: horizontalHeaderView
        anchors.left: parent.left
        anchors.leftMargin: verticalHeaderView.width
        width: parent.width
        height: 30
        model: ["Name", "Address", "Quant"]
        syncView: tableView
    }

    VerticalHeaderView {
        id: verticalHeaderView
        anchors.top: parent.top
        anchors.topMargin: 30
        height: parent.height
        syncView: tableView
    }

    TableView {
        id: tableView
        anchors.fill: parent
        anchors.leftMargin: verticalHeaderView.width
        anchors.topMargin: horizontalHeaderView.height
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
