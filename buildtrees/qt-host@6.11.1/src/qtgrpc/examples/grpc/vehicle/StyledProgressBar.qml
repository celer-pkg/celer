pragma ComponentBehavior: Bound

// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

ProgressBar {
    id: root

    property string activeColor: ""
    property string bgColor: ""

    height: 10

    background: Rectangle {
        anchors.fill: root
        radius: 2
        color: root.bgColor
    }

    contentItem: Item {
        // Progress indicator for determinate state.
        Rectangle {
            visible: !root.indeterminate
            width: root.visualPosition * root.width
            height: root.height
            radius: 2
            color: root.activeColor

            Behavior on width {
                NumberAnimation {
                    duration: 500
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Scrolling animation for indeterminate state.
        Item {
            visible: root.indeterminate
            anchors.fill: parent
            clip: true

            Row {
                spacing: 20

                Repeater {
                    model: root.width / 40 + 1

                    Rectangle {
                        color: root.activeColor
                        width: 20
                        height: parent.height
                    }
                }

                XAnimator on x {
                    from: 0
                    to: -40
                    loops: Animation.Infinite
                    running: root.indeterminate
                }
            }
        }
    }
}
