import QtQuick

Item {
    id: oldShape

    Path {
        id: oldPath

        startX: -100
        startY: 100

        PathCubic {
            x: 1000
            y: 200
            control1X: 1
            control1Y: 2
            control2X: 3
            control2Y: 4
        }
    }

    Instantiator {
        id: oldPathCubics

        delegate:
            PathCubic {
                x: 1000
                y: 200
                control1X: 1
                control1Y: 2
                control2X: 3
                control2Y: 4
            }

        onObjectAdded: (index, object) => {
            oldPath.pathElements.splice(index, 0, object)
            gc()
        }
        onObjectRemoved: (index, object) => {
            oldPath.pathElements.splice(index, 1)
        }
    }

    Timer {
        Component.onCompleted: start()
        interval: 2
        repeat: true
        property bool v: false
        onTriggered: {
            oldPathCubics.model = v ? 0 : 100
            v = !v
        }
    }
}
