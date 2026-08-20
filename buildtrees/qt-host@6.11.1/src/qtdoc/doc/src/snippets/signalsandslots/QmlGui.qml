// Copyright (C) 2014 Sze Howe Koh <szehowe.koh@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

//! [crosslanguage]
Rectangle {
    width: 100; height: 100

    signal qmlSignal(string sentMsg)
    function qmlSlot(receivedMsg) {
        console.log("QML received: " + receivedMsg)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: qmlSignal("Hello from QML!")
    }
}
//! [crosslanguage]

