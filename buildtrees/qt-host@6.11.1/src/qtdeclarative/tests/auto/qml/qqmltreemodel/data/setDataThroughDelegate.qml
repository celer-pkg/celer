// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt.labs.qmlmodels

Item {
    id: root
    width: 200
    height: 200

    property alias testModel: testModel

    signal shouldModify()
    signal shouldModifyInvalidRole()
    signal shouldModifyInvalidType()

    function modify() {
        shouldModify()
    }

    function modifyInvalidRole() {
        shouldModifyInvalidRole()
    }

    function modifyInvalidType() {
        shouldModifyInvalidType()
    }

    TreeView {
        anchors.fill: parent
        model: TestModel {
            id: testModel
        }

        Component.onCompleted: {
            expandRecursively()
        }

        delegate: Text {
            id: textItem
            text: model.display

            Connections {
                target: root
                enabled: column === 1
                function onShouldModify() { model.display = 18 }
            }

            Connections {
                target: root
                enabled: column === 0
                // Invalid: checked is not a role.
                function onShouldModifyInvalidRole() { model.checked = true }
            }

            Connections {
                target: root
                enabled: column === 1
                // Invalid: should be int.
                function onShouldModifyInvalidType() { model.display = "Whoops" }
            }
        }

    }
}