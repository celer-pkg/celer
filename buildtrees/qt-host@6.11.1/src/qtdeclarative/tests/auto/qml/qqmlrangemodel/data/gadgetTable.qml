// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

TableView {
    id: tableView
    width: 250
    height: 320
    columnSpacing: 10
    rowSpacing: 10

    required delegateModelAccess
    required model
    property var currentItem

    selectionModel: ItemSelectionModel {
    }

    Component.onCompleted: {
        selectionModel.setCurrentIndex(model.index(0, 0), ItemSelectionModel.SelectCurrent)
    }

    delegate: Rectangle {
        id: cell
        implicitWidth: 100
        implicitHeight: 25

        required property bool current
        Binding {
            when: cell.current
            tableView.currentItem: cell
        }

        required property var modelData
        required property string text
        property int number: modelData.number


        Text {
            anchors.fill: parent
            text: cell.text + ": " + cell.number
        }

        function setValue(value: string)
        {
            text = value;
        }

        function setModelData(value)
        {
            modelData = value;
        }

        function setModelDataNumber(number: int)
        {
            modelData.number = number;
        }
    }
}
