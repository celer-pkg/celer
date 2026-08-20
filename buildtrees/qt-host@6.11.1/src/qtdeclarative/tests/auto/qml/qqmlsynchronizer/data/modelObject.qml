// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQml
import Qt.labs.synchronizer
import Test

DelegateModel {
    id: root

    property ListModel listModel: ListModel {
        ListElement {
            a: "a"
            b: "b"
        }
    }

    model: listModel

    delegate: QtObject {
        objectName: "foo"

        required property QtObject model

        Synchronizer on objectName {
            targetObject: model
            targetProperty: "a"
        }
    }
}
