// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

//! [document]
Column {
    spacing: 2
    Rectangle { color: "red"; width: 50; height: 50 }
    Rectangle { id: rect1; color: "green"; width: 20; height: 50 }
    Rectangle { color: "blue"; width: 50; height: 20 }

    //![transitions]
    add: Transition {
        NumberAnimation { properties: "opacity"; duration: 1000 }
        }
    //![transitions]

    MouseArea {
        anchors.fill: parent
        onClicked: rect1.opacity = 0
    }
}
//! [document]
