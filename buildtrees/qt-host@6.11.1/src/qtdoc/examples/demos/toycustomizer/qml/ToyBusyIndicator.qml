// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic.impl
import QtQuick.Templates as T

Item {
    id: root

    implicitWidth: busyIndicator.implicitWidth
    implicitHeight: busyIndicator.implicitHeight

    Rectangle {
        anchors.fill: parent
        color: "#DAEBFF"
        opacity: 0.4
    }

    T.BusyIndicator {
        id: busyIndicator

        implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)
        padding: 6
        running: root.visible
        anchors.centerIn: parent

        background: Rectangle {
            color: "#DAEBFF"
            opacity: 0.7
            radius: width / 2
        }

        contentItem: BusyIndicatorImpl {
            implicitWidth: 68
            implicitHeight: 68
            pen: "#1C44B1"
            fill: "#1C44B1"
            running: busyIndicator.running
            opacity: busyIndicator.running ? 1 : 0
            Behavior on opacity { OpacityAnimator { duration: 250 } }
        }
    }
}
