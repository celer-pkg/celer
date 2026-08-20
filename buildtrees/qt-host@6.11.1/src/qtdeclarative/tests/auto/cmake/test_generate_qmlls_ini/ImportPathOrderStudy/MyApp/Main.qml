// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import MyLib

Window {
    width: 640
    height: 480
    visible: true

    QmlObj {
        id: obj
    }

    Component.onCompleted: {
        /*!!
            Expectations:
            - Only one of these lines will print correctly at runtime
            - All QML tools should flag the other line as [missing-property]
        */
        console.log(obj.realProp);
        console.log(obj.mockProp);
    }
}
