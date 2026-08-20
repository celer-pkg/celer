import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

View3D {
    width: 640
    height: 480
    id: view1
    anchors.fill: parent

    PerspectiveCamera {
        id: camera1
    }

    GridGeometry {
        id: gird1
        horizontalLines: 10
        verticalLines: 10
    }

    GridGeometry {
        id: grid2
        horizontalLines: 8
        verticalLines: 8
        horizontalStep: 0.1
        verticalStep: 0.1
    }

    GridGeometry {
        id: grid3
        horizontalLines: 18
        verticalLines: 18
        horizontalStep: 30.0
        verticalStep: 20.5
    }
}
