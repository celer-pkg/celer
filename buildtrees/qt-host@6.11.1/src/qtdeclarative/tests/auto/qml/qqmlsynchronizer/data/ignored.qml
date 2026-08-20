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
    }

    Synchronizer on count {
        property alias target: other.count
        onValueBounced: ++root.countBounces
        onValueIgnored: ++root.countIgnores
    }
}
