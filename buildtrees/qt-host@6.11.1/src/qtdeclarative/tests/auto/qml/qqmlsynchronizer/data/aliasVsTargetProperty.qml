// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQml
import Qt.labs.synchronizer

QtObject {
    id: root
    objectName: "bar"
    property string unbindable: "a"
    property int count: 12

    property int countBounces: 0
    property int countIgnores: 0

    property QtObject other: QtObject {
        id: other
        objectName: "foo"
        property string unbindable: "b"
        property double count: 13.5

        onCountChanged: count = 13.5 // reject any changes to count
    }

    property Synchronizer s1: Synchronizer {
        targetObject: root
        targetProperty: "objectName"
        property alias source: other.objectName
    }

    property Synchronizer s2: Synchronizer {
        targetObject: root
        targetProperty: "unbindable"
        property alias source: other.unbindable
    }

    property Synchronizer s3: Synchronizer {
        targetObject: root
        targetProperty: "count"
        property alias source: other.count
        onValueBounced: ++root.countBounces
        onValueIgnored: ++root.countIgnores
    }
}
