import QtQuick
import QtQuick.Shapes

Item {
    width: 820
    height: 850
    Item {
        x: 160
        y: 160

        implicitWidth: 100
        implicitHeight: 100

        scale: 5

        Item {
            implicitWidth: 100
            implicitHeight: 100

            Shape {
                implicitWidth: 80
                implicitHeight: 80
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeColor: "transparent"
                    fillColor: "red"
                    fillRule: ShapePath.WindingFill
                    PathSvg { path: "M 10 35 C 10 23.9543 18.9543 15 30 15 C 41.0457 15 50 23.9543 50 35 C 50 23.9543 58.9543 15 70 15 C 81.0457 15 90 23.9543 90 35 C 90 55 76.6667 75 50 95 C 23.3333 75 10 55 10 35 " }
                }
            }
        }

        Item {
            x: 80
            implicitWidth: 100
            implicitHeight: 100
            layer.enabled: true
            layer.textureSize: Qt.size(500, 500)

            Shape {
                implicitWidth: 80
                implicitHeight: 80
                preferredRendererType: Shape.CurveRenderer
                ShapePath {
                    strokeColor: "transparent"
                    fillColor: "red"
                    fillRule: ShapePath.WindingFill
                    PathSvg { path: "M 10 35 C 10 23.9543 18.9543 15 30 15 C 41.0457 15 50 23.9543 50 35 C 50 23.9543 58.9543 15 70 15 C 81.0457 15 90 23.9543 90 35 C 90 55 76.6667 75 50 95 C 23.3333 75 10 55 10 35 " }
                }
            }
        }

        Item {
            y: 80
            implicitWidth: 100
            implicitHeight: 100

            Shape {
                implicitWidth: 80
                implicitHeight: 80
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    strokeColor: "red"
                    fillRule: ShapePath.WindingFill
                    PathSvg { path: "M 10 35 C 10 23.9543 18.9543 15 30 15 C 41.0457 15 50 23.9543 50 35 C 50 23.9543 58.9543 15 70 15 C 81.0457 15 90 23.9543 90 35 C 90 55 76.6667 75 50 95 C 23.3333 75 10 55 10 35 " }
                }
            }
        }

        Item {
            y: 80
            x: 80
            implicitWidth: 100
            implicitHeight: 100
            layer.enabled: true
            layer.textureSize: Qt.size(500, 500)

            Shape {
                implicitWidth: 80
                implicitHeight: 80
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    strokeColor: "red"
                    fillRule: ShapePath.WindingFill
                    PathSvg { path: "M 10 35 C 10 23.9543 18.9543 15 30 15 C 41.0457 15 50 23.9543 50 35 C 50 23.9543 58.9543 15 70 15 C 81.0457 15 90 23.9543 90 35 C 90 55 76.6667 75 50 95 C 23.3333 75 10 55 10 35 " }
                }
            }
        }

    }
}
