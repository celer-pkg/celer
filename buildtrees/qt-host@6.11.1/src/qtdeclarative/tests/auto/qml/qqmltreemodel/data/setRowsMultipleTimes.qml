// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt.labs.qmlmodels

Item {
    id: root
    width: 200
    height: 200

    property alias testModel: testModel
    property alias treeView: treeView

    function setRowsInvalid() {
        testModel.rows = [
                            {
                                checked: false,
                                amount: 4,
                                fruitType: "Peach",
                                fruitName: "Princess Peach",
                                fruitPrice: 1.45,
                                color: "yellow",
                                rows: [
                                    {
                                        checked: true,
                                        amount: 5,
                                        fruitType: "Strawberry",
                                        fruitName: [],
                                        fruitPrice: 3.80,
                                        color: "red"
                                    },
                                    {
                                        checked: false,
                                        amount: 6,
                                        fruitType: "Pear",
                                        fruitName: "Bear Pear",
                                        fruitPrice: 1.50,
                                        color: "green"
                                    }
                                ]
                            }
                        ]
    }

    function setRows() {
        testModel.rows = [
                            {
                                checked: true,
                                amount: 5,
                                fruitType: "Strawberry",
                                fruitName: "Perry the Berry",
                                fruitPrice: 3.80,
                                color: "red",
                            },
                            {
                                checked: false,
                                amount: 6,
                                fruitType:"Pear",
                                fruitName: "Bear Pear",
                                fruitPrice: 1.50,
                                color: "green",
                            },
                            {
                                checked: true,
                                amount: 4,
                                fruitType: "Orange",
                                fruitName: "Navel",
                                fruitPrice: 2.50,
                                color: "orange"
                            }
                        ]
    }

    TreeView {
        id: treeView
        anchors.fill: parent
        model: TestModel {
            id: testModel
        }
        delegate: Text {
            text: model.display
        }
    }
}