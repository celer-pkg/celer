// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [file]
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Qt.labs.qmlmodels

TableView {
    width: 540
    height: 40 * rows

    columnWidthProvider: function(column) {
        switch (column) {
        case 0: return 220
        case 1: return 260
        case 2: return 60
        default: return -1
        }
    }

    //! [delegate]
    delegate: TableViewDelegate {
        id: tableCell

        checked: column === 0 ? checkBox.checked : tableView.itemAtIndex(tableView.index(row, 0)).checked
        selected: checked

        //! [background]
        background: Item {
            Rectangle {
                anchors.fill: parent
                anchors.margins: tableCell.current ? 3 : 1
                color: tableCell.selected ? "blue" : "white"
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: "darkblue"
                border.width: tableCell.current ? 2 : 0
            }
        }
        //! [background]

        //! [contentItem]
        contentItem: Item {
            implicitHeight: 40
            visible: !tableCell.editing

            RowLayout {
                anchors.fill: parent

                CheckBox {
                    id: checkBox
                    implicitWidth: height
                    Layout.fillHeight: true
                    checked: false
                    visible: tableCell.column === 0
                }

                Text {
                    Layout.leftMargin: 4
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    color: tableCell.selected ? "white" : "black"
                    text: tableCell.model.display
                }
            }
        }
        //! [contentItem]

        //! [editDelegate]
        TableView.editDelegate: FocusScope {
            width: parent.width
            height: parent.height

            TableView.onCommit: {
                let qaim = tableCell.tableView.model
                if (!qaim)
                    return
                const index = qaim.index(tableCell.row, tableCell.column)
                // instead of the edit role, any custom role supported by the model can be checked
                // e.g. if (!tableCell.checked || !tableCell.model.customRole)
                if (!tableCell.checked || !tableCell.model.edit)
                    return
                // instead of the edit role, any custom role supported by the model can be set
                // e.g. tableCell.model.customRole = textField.text
                tableCell.model.edit = textField.text
                tableCell.model.display = textField.text
            }

            Component.onCompleted: textField.selectAll()

            TextField {
                id: textField
                anchors.fill: parent
                text: tableCell.model.edit ?? tableCell.model.display ?? ""
                focus: true
            }
        }
        //! [editDelegate]
    }
    //! [delegate]

    model: TableModel {
        TableModelColumn { display: "name" }
        TableModelColumn { display: "address" }
        TableModelColumn { display: "quantity" }

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

    selectionModel: ItemSelectionModel { }
}
//! [file]
