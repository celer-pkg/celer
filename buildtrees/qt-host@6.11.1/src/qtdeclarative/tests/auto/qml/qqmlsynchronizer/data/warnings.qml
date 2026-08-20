// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQml
import Qt.labs.synchronizer
import Test

BindablePoint {
    id: root
    objectName: "bar"
    property string unbindable: "a"
    property int count: 12

    property int countBounces: 0
    property int countIgnores: 0

    function doThings() {}

    property QtObject other: QtObject {
        id: other
        objectName: "foo"
        property string unbindable: "b"
        property double count: 13.5
    }

    property Synchronizer mismatchedType: Synchronizer {
        targetObject: root
        targetProperty: "point"
        property alias source: root.other
    }

    property Synchronizer missingProperty: Synchronizer {
        targetObject: root
        targetProperty: "doesNotExist"
        property alias source: other.unbindable
    }

    property Synchronizer onFunction: Synchronizer {
        targetObject: root
        targetProperty: "doThings"
        property alias source: other.count
    }
}
