// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    width: 640
    height: 400
    color: "white"
    ColumnLayout {
        anchors.fill: parent
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "XR Touch Example"
        }
        RowLayout {
            ColumnLayout {
                Button {
                    text: "Button 1"
                    Layout.margins: 15
                }
                Button {
                    text: "Button 2"
                    Layout.margins: 15
                }
                Button {
                    text: "Button 3"
                    Layout.margins: 15
                }
                Slider {}
            }
            Rectangle {
                id: field
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "#ffeedd"
                MultiPointTouchArea {
                    anchors.fill: parent
                    mouseEnabled: true // For debug purposes
                    touchPoints: [
                        TouchPoint { id: point1 },
                        TouchPoint { id: point2 }
                    ]
                }
                Rectangle {
                    x: point1.x - 15
                    y: point1.y - 15
                    visible: point1.pressed
                    width: 30; height: 30
                    radius: 15
                    color: point1.pointId === 0 ? "green" : "red"
                }

                Rectangle {
                    x: point2.x - 15
                    y: point2.y - 15
                    visible: point2.pressed
                    width: 30; height: 30
                    radius: 15
                    color: point2.pointId === 0 ? "green" : "red"
                }


                Rectangle {
                    id: ball
                    width: 10; height: 10
                    radius: 5
                    color: "black"
                    property vector2d pos: Qt.vector2d(field.width / 2, field.height / 2)
                    property vector2d speed: Qt.vector2d(1, 1)
                    x: pos.x - radius / 2
                    y: pos.y - radius / 2
                    FrameAnimation {
                        running: true
                        onTriggered: {
                            // Touch points attract
                            const scale = 1.0 / Math.max(field.width, field.height)
                            let relativeAccel = Qt.vector2d(0, 0)
                            for (const point of [point1, point2]) {
                                if (point.pressed) {
                                    const vector = ball.pos.minus(Qt.vector2d(point.x, point.y)).times(scale)
                                    const dist = Math.max(100, vector.length() * 200)
                                    const a = vector.normalized().times(1 / (dist * dist))
                                    relativeAccel = relativeAccel.plus(a)
                                }
                            }
                            const accel = relativeAccel.times(1/scale)
                            ball.speed = ball.speed.minus(accel)

                            // Bounce off the edges with dampening
                            const nextX = ball.pos.x + ball.speed.x
                            if (nextX < 0 || nextX > field.width)
                                ball.speed.x = -ball.speed.x * 0.8
                            const nextY = ball.pos.y + ball.speed.y
                            if (nextY < 0 || nextY > field.height)
                                ball.speed.y = -ball.speed.y * 0.8
                            ball.pos = ball.pos.plus(ball.speed)
                        }
                    }
                }

                Text {
                    text: "Touch area"
                    anchors.centerIn: parent
                }
            }
        }
    }
}
