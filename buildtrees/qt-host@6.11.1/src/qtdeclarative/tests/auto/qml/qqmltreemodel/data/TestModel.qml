// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import Qt.labs.qmlmodels

TreeModel {
    id: treeModel
    objectName: "testModel"

    TableModelColumn { display: "checked"; decoration: "color" }
    TableModelColumn { display: "amount"; decoration: "color" }
    TableModelColumn { display: "fruitType"; decoration: "color" }
    TableModelColumn { display: "fruitName"; decoration: "color" }
    TableModelColumn { display: "fruitPrice"; decoration: "color" }

    rows: [
        {
            checked: false,
            amount: 1,
            fruitType: "Apple",
            fruitName: "Granny Smith",
            fruitPrice: 1.50,
            color: "red",
            rows: [
                {
                    checked: true,
                    amount: 4,
                    fruitType: "Orange",
                    fruitName: "Navel",
                    fruitPrice: 2.50,
                    color: "orange"
                },
                {
                    checked: false,
                    amount: 1,
                    fruitType: "Banana",
                    fruitName: "Cavendish",
                    fruitPrice: 3.50,
                    color: "yellow",
                    rows: [
                        {
                            checked: true,
                            amount: 4,
                            fruitType: "Orange",
                            fruitName: "Navel",
                            fruitPrice: 2.50,
                            color: "orange",
                        },
                        {
                            checked: false,
                            amount: 1,
                            fruitType: "Banana",
                            fruitName: "Cavendish",
                            fruitPrice: 3.50,
                            color: "yellow",
                        }
                    ]
                }
            ]
        },
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
                    fruitName: "Perry the Berry",
                    fruitPrice: 3.80,
                    color: "red",
                },
                {
                    checked: false,
                    amount: 6,
                    fruitType: "Pear",
                    fruitName: "Bear Pear",
                    fruitPrice: 1.50,
                    color: "green",
                }
            ]
        }
    ]
}
