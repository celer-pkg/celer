import QtQuick
import QtQuick.Effects

Rectangle {
    width: 320
    height: 500
    color: "#c0c0c0"

    // Helper component
    component Rect : Rectangle {
        width: 60
        height: 60
        color: "#909000"
        border.width: 4
        border.color: "#f0f0f0"
        radius: 10
    }

    // Plain shadow
    Rect {
        x: 20
        y: 20
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
        }
    }

    // blur
    Rect {
        x: 120
        y: 20
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            blur: 20
        }
    }

    // spread
    Rect {
        x: 220
        y: 20
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            spread: 10
        }
    }

    // spread + blur + color
    Rect {
        x: 20
        y: 120
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            spread: 10
            blur: 20
            color: "#8080e0"
        }
    }

    // offset + blur + opacity
    Rect {
        x: 120
        y: 120
        opacity: 0.5
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            offset.x: -10
            offset.y: -5
            blur: 20
        }
    }

    // 2 * offset + blur + color
    Rect {
        x: 220
        y: 120
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            offset.x: -10
            offset.y: -10
            blur: 20
            color: "#ffffff"
        }
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            offset.x: 10
            offset.y: 10
            blur: 20
            color: "#606060"
        }
    }

    // offset + negative spread
    Rect {
        x: 20
        y: 220
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            offset: Qt.vector2d(30, 15)
            spread: -10
            blur: 20
        }
    }

    // offset + rotation
    Rect {
        x: 120
        y: 220
        rotation: 45
        RectangularShadow {
            z: -1
            anchors.fill: parent
            radius: parent.radius
            offset: Qt.vector2d(10, 0)
        }
    }

    // shadows in shadows + color + opacity
    RectangularShadow {
        x: 220
        y: 220
        width: 60
        height: 60
        color: "#60ff0000"
        radius: 10
        RectangularShadow {
            anchors.fill: parent
            anchors.margins: 5
            radius: 20
            color: "yellow"
            opacity: 0.5
            RectangularShadow {
                anchors.centerIn: parent
                width: 20
                height: 20
                radius: 0
                color: "#000000"
            }
        }
    }

    // round
    RectangularShadow {
        x: 20
        y: 320
        width: 60
        height: 60
        radius: 30
        blur: 0.5
    }

    // round + blur
    RectangularShadow {
        x: 120
        y: 320
        width: 60
        height: 60
        radius: 30
        blur: 20
    }

    // box
    RectangularShadow {
        x: 220
        y: 320
        width: 60
        height: 60
        radius: 0
        blur: 0.5
    }

    // box + blur
    RectangularShadow {
        x: 20
        y: 420
        width: 60
        height: 60
        radius: 0
        blur: 20
    }

    // round bar, Rectangle inside
    RectangularShadow {
        x: 120
        y: 420
        width: 160
        height: 60
        radius: 30
        Rectangle {
            anchors.fill: parent
            anchors.margins: 10
            radius: parent.radius
        }
    }
}
