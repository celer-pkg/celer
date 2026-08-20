// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

SplitView {
    id: mainSplitView

    handle: Rectangle {
        implicitWidth: parent.orientation === Qt.Horizontal ? 6 : parent.width
        implicitHeight: parent.orientation === Qt.Horizontal ? parent.height : 6
        color: Qt.lighter(mainView.backgroundColor1, 1.2)
        Image {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            source:"images/icon_viewseparator.png"
            rotation:  parent.parent.orientation === Qt.Horizontal ? 90 : 0
            opacity: parent.SplitHandle.pressed ? 1.0 : parent.SplitHandle.hovered ? 0.6 : 0.2
            width: 32
            height: 8
            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }
        }
    }
}
