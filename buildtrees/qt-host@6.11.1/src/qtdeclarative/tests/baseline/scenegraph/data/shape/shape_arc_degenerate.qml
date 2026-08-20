import QtQuick
import QtQuick.Shapes

Item {
    width: 320
    height: 480

    ListModel {
        id: renderers
        ListElement { renderer: Shape.GeometryRenderer }
        ListElement { renderer: Shape.CurveRenderer }
    }

    Row {
        Repeater {
            model: renderers
            Column {
                Item {
                    width: 160
                    height: 100

                    Shape {
                        anchors.fill: parent
                        preferredRendererType: renderer

                        ShapePath {
                            fillColor: "transparent"
                            strokeColor: "red"
                            strokeStyle: ShapePath.DashLine
                            strokeWidth: 4

                            startX: 4; startY: 4
                            PathArc {
                                x: 96; y: 96
                                radiusX: 0; radiusY: 0
                            }
                        }
                    }

                    Shape {
                        anchors.fill: parent
                        preferredRendererType: renderer

                        ShapePath {
                            fillColor: "transparent"
                            strokeColor: "blue"
                            strokeStyle: ShapePath.DashLine
                            strokeWidth: 2

                            PathSvg {
                                path: "M 4 4 A 0 0 0 0 1 96 96"
                            }
                        }
                    }

                }

                Item {
                    width: 160
                    height: 100

                    Repeater {
                        model: 2
                        Shape {
                            anchors.fill: parent
                            preferredRendererType: renderer

                            ShapePath {
                                fillColor: "transparent"
                                strokeColor: model.index === 0 ? "red" : "blue"
                                strokeStyle: ShapePath.DashLine
                                strokeWidth: model.index === 0 ? 4 : 2

                                startX: 10; startY: 100
                                PathArc {
                                    x: 60; y: 150
                                    radiusX: model.index === 0 ? 0 : 50
                                    radiusY: model.index === 0 ? 50 : 0
                                }
                            }
                        }
                    }
                }

                Item {
                    width: 160
                    height: 100

                    Repeater {
                        model: 2
                        Shape {
                            anchors.fill: parent
                            preferredRendererType: renderer

                            ShapePath {
                                fillColor: "transparent"
                                strokeColor: model.index === 0 ? "red" : "blue"
                                strokeStyle: ShapePath.DashLine
                                strokeWidth: model.index === 0 ? 4 : 2

                                PathSvg {
                                    path: model === 0
                                          ? "M 30 150 A 0 50 0 0 1 130 150"
                                          : "M 30 150 A 50 0 0 0 1 130 150"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
