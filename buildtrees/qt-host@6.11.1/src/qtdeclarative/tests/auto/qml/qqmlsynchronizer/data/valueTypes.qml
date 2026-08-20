// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQml
import Qt.labs.synchronizer
import Test

BindablePoint {
    id: root
    objectName: "11"
    property int count: 12

    property alias aa: root.point.x

    property QtObject other: QtObject {
        id: other
        objectName: "foo"
        property double count: 13.5
        property point point: ({x: 103, y: 104})
    }

    Synchronizer on count {
        property alias target: other.point.x
    }

    Synchronizer on objectName {
        property alias target: other.point.y
    }

    property Synchronizer s1: Synchronizer {
        targetObject: other
        targetProperty: "count"
        property alias source: root.point.x
    }

    property Synchronizer s2: Synchronizer {
        property alias source: root.point.y
        property alias target: other.objectName
    }
}
