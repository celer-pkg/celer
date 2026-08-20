import QtQuick
import QtQuick.Shapes

Rectangle {
    width: 320
    height: 480
    color: "lightyellow"

    ListModel {
        id: renderers
        ListElement { renderer: Shape.GeometryRenderer }
        ListElement { renderer: Shape.CurveRenderer }
    }

    Row {
        Repeater {
            model: renderers
            Column {
                Repeater {
                    model: 4
                    Item {
                        width: 160
                        height: 120

                        Shape {
                            id: shape1
                            anchors.fill: parent
                            preferredRendererType: renderer

                            ShapePath {
                                id: path2
                                strokeWidth: 5
                                strokeColor: "blue"
                                fillColor: "transparent"
                                capStyle: ShapePath.RoundCap
                                joinStyle: ShapePath.RoundJoin

                                PathAngleArc {
                                    centerX: shape1.width / 2
                                    centerY: shape1.height / 2
                                    radiusX: centerX - 10
                                    radiusY: centerY - 10
                                    sweepAngle: 360
                                }

                                trim.start: 0.1
                                trim.end: 0.9
                            }

                            ShapePath {
                                id: path1
                                strokeWidth: 5
                                strokeColor: "black"
                                fillColor: "orange"

                                startX: 30; startY: 100
                                PathCubic {
                                    x: 130; y: 100
                                    control1X: 30; control1Y: 15
                                    control2X: -30; control2Y: 30
                                }
                                PathLine {
                                    x: 50; y: 80
                                }
                                PathQuad {
                                    x: path1.startX; y: path1.startY
                                    relativeControlX: 30
                                    relativeControlY: 15
                                }

                            }

                            Timer {
                                running: true
                                interval: 50
                                onTriggered: {
                                    path1.trim.start = model.index * 0.2
                                    path1.trim.end = 1 - model.index * 0.2

                                    path2.trim.offset = model.index * -0.3
                                }
                            }


                        }
                    }
                }
            }
        }
    }
}
