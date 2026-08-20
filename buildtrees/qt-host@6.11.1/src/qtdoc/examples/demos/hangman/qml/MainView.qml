// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import examples.Hangman

Item {
    id: topLevel

    height: 480
    width: 320
    property real buttonHeight: 0
    property real globalMargin: width * 0.025

    Component.onCompleted: forceActiveFocus()
    focus: true
    Keys.onBackPressed: {
        if (pageStack.depth === 1) {
            Qt.quit()
        } else {
            pageStack.pop()
            event.accepted = true
            forceActiveFocus()
        }
    }

    Hangman {
        id: applicationData
        onWordChanged: {
            if (word.length > 0)
                splashLoader.source = "";
        }
    }

    StackView {
        id: pageStack
        anchors.fill: topLevel
        initialItem: "GameView.qml"
    }
    // ![0]
    Store {
        id: iapStore
    }
    // ![0]
}
