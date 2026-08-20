import QtQuick
import QtQuick.Shapes

Item {
    width: 600
    height: 600

    ListModel {
        id: renderers
        ListElement { renderer: Shape.CurveRenderer; fillrule: ShapePath.OddEvenFill }
        ListElement { renderer: Shape.CurveRenderer; fillrule: ShapePath.WindingFill }
        ListElement { renderer: Shape.GeometryRenderer; fillrule: ShapePath.OddEvenFill }
        ListElement { renderer: Shape.GeometryRenderer; fillrule: ShapePath.WindingFill }
    }

    Column {
        anchors.centerIn: parent
        spacing: 25

        Repeater {
            model: renderers

            Shape {
                preferredRendererType: renderer

                ShapePath {
                    fillColor: "teal"
                    strokeColor: "purple"
                    joinStyle: ShapePath.RoundJoin
                    capStyle: ShapePath.RoundCap

                    strokeWidth: 4
                    fillRule: fillrule

                    PathSvg {
                        // Single path, self-intersecting, same direction hole
                        path: "M 10,5 l 100,10 -80,80 0,-60 80,80 -100,10 z"
                    }

                    PathSvg {
                        // Single path, opposite direction hole
                        path: "M 150,5 l 100,10 -50,50 -30,-30 0,60 30,-30 50,50 -100,10 z"
                    }

                    PathSvg {
                        // Two subpaths, same direction hole
                        path: "M 290,5 l 100,10 -50,50 50,50 -100,10 z m 50,60 l -30,30 0,-60 z"
                    }

                    PathSvg {
                        // Two subpaths, opposite direction hole
                        path: "M 430,5 l 100,10 -50,50 50,50 -100,10 z m 50,60 l -30,-30 0,60 z"
                    }
                }
            }
        }
    }
}
