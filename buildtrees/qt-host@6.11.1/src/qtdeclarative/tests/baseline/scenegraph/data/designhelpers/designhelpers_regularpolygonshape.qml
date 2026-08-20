import QtQuick
import QtQuick.Shapes
import QtQuick.Shapes.DesignHelpers

Rectangle {
    id: root
    width: 800
    height: 800
    color:"#b5e7a0"

    component TestRegularPolygonShape : RegularPolygonShape {
        fillColor: "#37c1ff"
        strokeColor: "#663333"
        width: 90
        height: 90
    }

    Flow {
        spacing: 8
        anchors.margins: 6
        anchors.fill: parent
        TestRegularPolygonShape {} // default
        TestRegularPolygonShape {
            cornerRadius: 0
            strokeColor: "red"
            fillColor: "transparent"
        }
        TestRegularPolygonShape { sideCount: 3 }
        TestRegularPolygonShape { sideCount: 4 }
        TestRegularPolygonShape { sideCount: 5 }
        TestRegularPolygonShape { sideCount: 7 }
        TestRegularPolygonShape { sideCount: 8 }
        TestRegularPolygonShape { sideCount: 9 }

        TestRegularPolygonShape { sideCount: 10 }
        TestRegularPolygonShape { cornerRadius: 20 }
        TestRegularPolygonShape { cornerRadius: 30 }
        TestRegularPolygonShape { cornerRadius: 40 }
        TestRegularPolygonShape { cornerRadius: 50 }
        TestRegularPolygonShape { strokeWidth: 0; }
        TestRegularPolygonShape { strokeWidth: 1; }
        TestRegularPolygonShape { strokeWidth: 2; }

        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            cornerRadius: 4
        }
        TestRegularPolygonShape { strokeStyle: ShapePath.DashLine }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.SquareCap
        }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.FlatCap
        }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.RoundCap
        }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            dashOffset: 2
        }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            dashOffset: 6
        }
        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            dashPattern: [1,2]
        }

        TestRegularPolygonShape {
            strokeStyle: ShapePath.DashLine
            dashPattern: [2,4]
        }
        TestRegularPolygonShape {
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
        TestRegularPolygonShape {
            id: polygon1
            fillGradient: RadialGradient {
                focalX: polygon1.width * 0.5
                focalY: polygon1.height * 0.5
                centerX: polygon1.width * 0.5
                centerY: polygon1.height * 0.5
                centerRadius: polygon1.width * 0.5
                GradientStop { position:0.1; color:"cyan" }
                GradientStop { position:0.2; color:"green" }
                GradientStop { position:0.4; color:"red" }
                GradientStop { position:0.6; color:"yellow" }
                GradientStop { position:1.0; color:"blue" }
            }
        }
        TestRegularPolygonShape {
            strokeWidth: 6
            joinStyle: ShapePath.MiterJoin
            strokeStyle: ShapePath.DashLine
        }
        TestRegularPolygonShape {
            strokeWidth: 6
            joinStyle: ShapePath.BevelJoin
            strokeStyle: ShapePath.DashLine
        }
        TestRegularPolygonShape {
            cornerRadius: 0
            strokeWidth: 6
            joinStyle: ShapePath.RoundJoin
            strokeStyle: ShapePath.DashLine
        }
    }
}
