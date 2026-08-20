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

    function changeFruitTypeIntOverload() {
        var idx = testModel.index([1], 2)
        testModel.setData(idx, "Ananas", Qt.DisplayRole)
    }

    function changeFruitNameStringOverload() {
        var idx = testModel.index([1], 3)
        testModel.setData(idx, "My other favorite fruit", "display")
    }

    function changeFruitType(idx, value, role) {
        testModel.setData(idx, value, role)
    }

    function changeFruitName(idx, value, role) {
        testModel.setData(idx, value, role)
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
