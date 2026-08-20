// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import TestModel

Item {
    width: 800
    height: 600

    property alias treeView: treeView
    property int rowToExpand: -1

    TreeView {
        id: treeView
        anchors.fill:parent
        anchors.margins: 10
        model: TestModel {}
        selectionModel: ItemSelectionModel {}
        clip: true
        delegate: TreeViewDelegate {
            implicitWidth: treeView.width
            onHasChildrenChanged: {
                if (row == rowToExpand && hasChildren)
                   treeView.expand(row)
            }
        }
    }
}
