// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Shapes

Rectangle {
    function addAnswer(answer: string): void  {animatedAnswer.addAnswer(answer)}
    function startWaiting(): void  {animatedAnswer.startWaiting()}
    function cancelAnimation(): void  {animatedAnswer.cancelAnimation()}
    property alias canRequestAnswer: animatedAnswer.canRequestAnswer

    implicitWidth: 433
    implicitHeight: width

    color: "black"
    radius: 300
    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop {
            position: 0.0
            color: "#4b4b4b"
        }
        GradientStop {
            position: 0.33
            color: "#212121"
        }
        GradientStop {
            position: 1.0
            color: "#000000"
        }
    }

    // Reflection decoration
    Rectangle {
        width: 300
        height: width

        anchors.centerIn: parent
        anchors.horizontalCenterOffset: 6
        radius: 300

        color: "#bababa"
    }

    // Ball center
    Rectangle {
        anchors.centerIn: parent

        width: 300
        height: width

        color: "black"
        border.width: 1.5
        border.color: "#bababa"
        radius: 300

        Shape {
            id: ballTriangle
            anchors.centerIn: parent
            width: 250
            height: 250
            ShapePath {
                strokeWidth: 4
                strokeColor: "#213f94"
                capStyle: ShapePath.RoundCap

                fillGradient: RadialGradient {
                    centerX: ballTriangle.width / 2
                    centerY: ballTriangle.height / 2
                    focalX: centerX
                    focalY: centerY
                    centerRadius: 50
                    focalRadius: 0

                    GradientStop {
                        position: 0
                        color: "#1C2F60"
                    }
                    GradientStop {
                        position: 0.5
                        color: "#000547"
                    }
                    GradientStop {
                        position: 1
                        color: "#000324"
                    }
                }

                startX: 26
                startY: 68

                PathLine {
                    x: 125
                    y: 230
                }
                PathLine {
                    x: 224
                    y: 68
                }
                PathLine {
                    x: 26
                    y: 68
                }
            }
        }

        AnimatedAnswer {
            id: animatedAnswer
            anchors.centerIn: parent
        }
    }
}
