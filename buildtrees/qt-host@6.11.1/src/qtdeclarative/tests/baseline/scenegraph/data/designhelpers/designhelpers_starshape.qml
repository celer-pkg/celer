import QtQuick
import QtQuick.Shapes
import QtQuick.Shapes.DesignHelpers

Rectangle {
    id: root
    width: 800
    height: 800
    color:"#b5e7a0"

    component TestStarShape : StarShape {
        fillColor: "#37c1ff"
        strokeColor: "#663333"
        width: 90
        height: 90
    }

    Flow {
        spacing: 8
        anchors.margins: 6
        anchors.fill: parent
        TestStarShape {} // default
        TestStarShape {
            cornerRadius: 0
            strokeColor: "red"
            fillColor: "transparent"
        }
        TestStarShape { pointCount: 3 }
        TestStarShape { pointCount: 4; cornerRadius: 0 }
        TestStarShape { pointCount: 5; cornerRadius: 0 }
        TestStarShape { pointCount: 6; cornerRadius: 20 }
        TestStarShape { pointCount: 7 }
        TestStarShape { pointCount: 8; ratio: 0.7 }
        TestStarShape { pointCount: 9; ratio: 0.8 }
        TestStarShape { pointCount: 10; ratio: 0.9 }
        TestStarShape { pointCount: 20; ratio: 1.0 }
        TestStarShape {
            pointCount: 30
            cornerRadius: 20
            strokeWidth: 1
        }
        TestStarShape {
            pointCount: 40
            strokeWidth: 1
        }
        TestStarShape {
            pointCount: 50
            cornerRadius: 0
            strokeWidth: 1
        }
        TestStarShape {
            pointCount: 60
            cornerRadius: 0
            strokeWidth: 1
            ratio: 0.7
        }
        TestStarShape { pointCount: 3; strokeWidth: 0 }
        TestStarShape { pointCount: 4; strokeWidth: 1 }
        TestStarShape { pointCount: 5; strokeWidth: 2 }

        TestStarShape {
            strokeStyle: ShapePath.DashLine
            cornerRadius: 4
        }
        TestStarShape { strokeStyle: ShapePath.DashLine }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.SquareCap
        }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.FlatCap
        }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.RoundCap
        }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            dashOffset: 2
        }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            dashOffset: 6
        }
        TestStarShape {
            strokeStyle: ShapePath.DashLine
            dashPattern: [1,2]
        }

        TestStarShape {
            strokeStyle: ShapePath.DashLine
            dashPattern: [2,4]
        }
        TestStarShape {
            fillGradient: LinearGradient {
                x1: 20
                y1: 20
                x2: 100
                y2: 100
                GradientStop { position: 0.0; color: "red" }
                GradientStop { position: 0.33; color: "yellow" }
                GradientStop { position: 1.0; color: "green" }
            }
        }
        TestStarShape {
            id: star1
            fillGradient: RadialGradient {
                focalX: star1.width * 0.5
                focalY: star1.height * 0.5
                centerX: star1.width * 0.5
                centerY: star1.height * 0.5
                centerRadius: star1.width * 0.5
                GradientStop { position:0.1; color:"cyan" }
                GradientStop { position:0.2; color:"green" }
                GradientStop { position:0.4; color:"red" }
                GradientStop { position:0.6; color:"yellow" }
                GradientStop { position:1.0; color:"blue" }
            }
        }
        TestStarShape {
            strokeWidth: 6
            joinStyle: ShapePath.MiterJoin
            strokeStyle: ShapePath.DashLine
        }
        TestStarShape {
            strokeWidth: 6
            joinStyle: ShapePath.BevelJoin
            strokeStyle: ShapePath.DashLine
        }
        TestStarShape {
            cornerRadius: 0
            strokeWidth: 6
            joinStyle: ShapePath.RoundJoin
            strokeStyle: ShapePath.DashLine
        }
    }
}
