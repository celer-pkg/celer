import QtQuick
import QtQuick.Shapes

Item {
    width: 320
    height: 480

    Rectangle {
        x: 10
        y: 10
        width: 100
        height: 100.5
        layer.enabled: false
        color: "cornsilk"
        border.width: 1
        border.color: "black"
    }

    Rectangle {
        x: 10
        y: 120
        width: 100
        height: 100.5
        layer.enabled: true
        color: "oldlace"
        border.width: 1
        border.color: "black"
    }

    Rectangle {
        x: 10
        y: 240
        width: 100
        height: 100.5
        layer.enabled: true
        layer.samples: 4
        color: "linen"
        border.width: 1
        border.color: "black"
    }


    Shape {
        layer.enabled: true
        layer.samples: 4
        x: 150
        y: 100 + (0.5 / Screen.devicePixelRatio)

        ShapePath {
            strokeColor: "#FF0000"
            strokeWidth: 1 / Screen.devicePixelRatio
            capStyle: ShapePath.FlatCap
            startX: 0
            startY: 0
            PathLine {
                x: 100
                y: 0
            }
        }

        ShapePath {
            strokeColor: "#00FF00"
            strokeWidth: 1 / Screen.devicePixelRatio
            capStyle: ShapePath.FlatCap
            startX: 0
            startY: 34
            PathLine {
                x: 100
                y: 34
            }
        }
    }

    Shape {
        layer.enabled: true
        layer.samples: 4
        x: 150
        y: 200

        ShapePath {
            strokeColor: "#FF0000"
            strokeWidth: 1 / Screen.devicePixelRatio
            capStyle: ShapePath.FlatCap
            startX: 0
            startY: 0
            PathLine {
                x: 100
                y: 0
            }
        }

        ShapePath {
            strokeColor: "#00FF00"
            strokeWidth: 1 / Screen.devicePixelRatio
            capStyle: ShapePath.FlatCap
            startX: 0
            startY: 34
            PathLine {
                x: 100
                y: 34
            }
        }
    }
}
