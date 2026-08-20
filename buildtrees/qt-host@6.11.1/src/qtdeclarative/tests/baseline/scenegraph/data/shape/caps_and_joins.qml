// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Shapes

Item {
    width: 1500; height: 1350

    component Shapes: Shape {
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathMove { x: 160; y: 100 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            capStyle: ShapePath.SquareCap
            joinStyle: ShapePath.MiterJoin
            PathMove { x: 160; y: 450 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.BevelJoin
            PathMove { x: 160; y: 800 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }

        // cosmetic stroke, scale = 1
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            cosmeticStroke: true
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathMove { x: 410; y: 100 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            cosmeticStroke: true
            capStyle: ShapePath.SquareCap
            joinStyle: ShapePath.MiterJoin
            PathMove { x: 410; y: 450 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 10
            cosmeticStroke: true
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.BevelJoin
            PathMove { x: 410; y: 800 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: -25; relativeControlY: -25 }
            PathQuad { relativeX: 0; relativeY: -50;
                relativeControlX: 50; relativeControlY: -25 }
            PathCubic { relativeX: 0; relativeY: 300;
                relativeControl1X: -250; relativeControl1Y: 100
                relativeControl2X: 250; relativeControl2Y: 200 }
            PathLine { relativeX: -150; relativeY: -250 }
            PathLine { relativeX: 75; relativeY: 200 }
            PathLine { relativeX: -75; relativeY: 0 }
        }
    }
    Shapes {
        x: 100
        y: 200
        scale: 1.25
    }
    Shapes {
        x: 700
    }
    Shapes {
        x: 1100
        y: 100
        scale: 0.5
    }
    Shapes {
        x: 1100
        y: -300
        scale: 0.25
    }
}
