// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [example]
import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    width: 180
    height: 100

    Row {
        anchors.centerIn: parent
        spacing: 16

        Rectangle {
            id: normalRect
            width: 60
            height: 60
            color: "black"
            radius: 10
            layer.enabled: true
            layer.effect: Glow {
                samples: 15
                color: "blue"
                transparentBorder: false
            }
        }

        Rectangle {
            id: transparentBorderRect
            width: 60
            height: 60
            color: "black"
            radius: 10
            layer.enabled: true
            layer.effect: Glow {
                samples: 15
                color: "blue"
                transparentBorder: true
            }
        }
    }
}
//! [example]
