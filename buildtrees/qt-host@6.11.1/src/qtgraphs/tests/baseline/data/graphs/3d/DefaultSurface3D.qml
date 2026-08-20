import QtGraphs
import QtQuick
import QtQuick3D

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    ListModel {
        id: dataModel
        ListElement{ xPos: 0; yPos: 0; zPos: 0 }
        ListElement{ xPos: 1; yPos: -1; zPos: 0 }
        ListElement{ xPos: 2; yPos: 0.2; zPos: 0 }
        ListElement{ xPos: 3; yPos: -0.2; zPos: 0 }
        ListElement{ xPos: 0; yPos: 0.1; zPos: 1 }
        ListElement{ xPos: 1; yPos: -0.2; zPos: 1 }
        ListElement{ xPos: 2; yPos: -0.1; zPos: 1 }
        ListElement{ xPos: 3; yPos: 0; zPos: 1 }
        ListElement{ xPos: 0; yPos: 0.2; zPos: 2 }
        ListElement{ xPos: 1; yPos: 0.1; zPos: 2 }
        ListElement{ xPos: 2; yPos: 0; zPos: 2 }
        ListElement{ xPos: 3; yPos: 0.1; zPos: 2 }
        ListElement{ xPos: 0; yPos: -0.1; zPos: 3 }
        ListElement{ xPos: 1; yPos: -0.2; zPos: 3 }
        ListElement{ xPos: 2; yPos: -0.1; zPos: 3 }
        ListElement{ xPos: 3; yPos: 0.1; zPos: 3 }
    }

    Surface3D {
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.MixSeries
        }

        anchors.fill: parent
        cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh
        environment: SceneEnvironment {}

        Surface3DSeries {
            ItemModelSurfaceDataProxy {
                itemModel: dataModel
                columnRole: "xPos"
                yPosRole: "yPos"
                rowRole: "zPos"
            }
        }
    }
}
