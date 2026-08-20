// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtScxml

Item {
    id: root
    required property StateMachine anywhere

    Button {
        id: here
        enabled: root.anywhere ? root.anywhere.here : false
        text: "Go There"
        width: parent.width / 2
        height: parent.height
        onClicked: root.anywhere.submitEvent("goThere")
    }

    Button {
        id: there
        enabled: root.anywhere ? root.anywhere.there : false
        text: "Go Here"
        width: parent.width / 2
        height: parent.height
        x: width
        onClicked: root.anywhere.submitEvent("goHere")
    }
}
