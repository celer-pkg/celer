import QtQuick
import QtQuick.Shapes
import QtQuick.Shapes.DesignHelpers

Item {
    width: 800
    height: 800

    ControlledShape {
        delegate: [
            PathMove { x: p1.cx + 70; y: p1.cy },
            PathLine { x: p2.cx - 50; y: p2.cy },
            PathArc { x: p2.cx; y: p2.cy + 50; radiusX: 50; radiusY: 50 },
            PathLine { x: p3.cx; y: p3.cy - 50 },
            PathArc { x: p3.cx - 50; y: p3.cy; radiusX: 50; radiusY: 50 },
            PathLine { x: p4.cx + 50; y: p4.cy },
            PathArc { x: p4.cx; y: p4.cy - 50; radiusX: 50; radiusY: 50 },
            PathLine { x: p1.cx; y: p1.cy + 70 },
            PathLine { x: p1.cx + 70; y: p1.cy }
        ]

        ControlPoint {
            id: p1
            cx: 100
            cy: 100
        }
        ControlPoint {
            id: p2
            cx: 600
            cy: 100
        }
        ControlPoint {
            id: p3
            cx: 600
            cy: 400
        }
        ControlPoint {
            id: p4
            cx: 100
            cy: 400
        }
    }

    component RectangleShapeBase : RectangleShape {
        width: 300
        height: 200
        opacity: 0.5
    }

    Flow {
        spacing: 10
        anchors.fill: parent
        anchors.margins: 10
        anchors.topMargin: parent.height / 2
        z: -1

        // joinStyle
        Repeater {
            model: joinStyles

            readonly property var joinStyles: [ ShapePath.MiterJoin, ShapePath.BevelJoin, ShapePath.RoundJoin ]

            RectangleShapeBase {
                topLeftBevel: true
                strokeWidth: 20
                topLeftRadius: 40
                joinStyle: modelData

                required property int modelData
            }
        }
    }
}

