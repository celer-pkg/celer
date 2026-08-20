// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

Item {
    implicitWidth: 920
    implicitHeight: 240
    Shape {
        y: 30
        transform: Scale { xScale: xscale.value; yScale: yscale.value }
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: "#ff000000"
            strokeWidth: 2
            cosmeticStroke: true
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.MiterJoin
            miterLimit: 4
            fillColor: "#00000000"
            fillRule: ShapePath.WindingFill
            PathSvg { id: thePath; path: "M 10 20 L 14 20 L 14 24 L 40 100 Q 120 150 39 200 Z" }
        }
    }
    Text {
        x: 4
        anchors { top: toolbar.bottom }
        text: "cosmetic\n2px, scaled"
    }
    Shape {
        transform: Matrix4x4 { matrix: PlanarTransform.fromAffineMatrix(xscale.value, 0, 0, yscale.value, 100, 30) }
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: "#ff000000"
            strokeWidth: 2
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.MiterJoin
            miterLimit: 4
            fillColor: "#00000000"
            fillRule: ShapePath.WindingFill
            PathSvg { path: thePath.path }
        }
    }
    Text {
        x: 120
        anchors { top: toolbar.bottom }
        text: "2px, scaled"
    }
    Shape {
        transform: Translate { x: 300; y: 30 }
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: "#ff000000"
            strokeWidth: 2
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.MiterJoin
            miterLimit: 4
            fillColor: "#00000000"
            fillRule: ShapePath.WindingFill
            PathSvg { path: thePath.path }
        }
    }
    Text {
        x: 315
        anchors { top: toolbar.bottom }
        text: "2px, normal"
    }

    Row {
        id: toolbar
        spacing: 10
        Slider {
            id: xscale
            from: 1
            to: 10
        }
        Text {
            text: "x scale: " + xscale.value.toFixed(2)
        }

        Slider {
            id: yscale
            from: 1
            to: 10
        }
        Text {
            text: "y scale: " + yscale.value.toFixed(2)
        }
    }
}
