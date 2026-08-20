import QtGraphs
import QtQuick
import QtQuick3D

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    ListModel {
        id: dataModel
        ListElement{ xPos: -10.0; yPos: 5.0; zPos: -5.0 }
        ListElement{ xPos: -9.0; yPos: 3.0; zPos: -4.5 }
        ListElement{ xPos: -8.5; yPos: 4.1; zPos: -4.0 }
        ListElement{ xPos: -8.0; yPos: 4.75; zPos: -3.9 }
        ListElement{ xPos: -9.5; yPos: 4.9; zPos: -4.2 }
        ListElement{ xPos: -9.9; yPos: 3.42; zPos: -3.5 }
        ListElement{ xPos: -7.8; yPos: 3.1; zPos: -4.9 }
        ListElement{ xPos: -7.3; yPos: 2.91; zPos: -4.1 }
        ListElement{ xPos: -7.1 ; yPos: 3.68 ; zPos: -4.52 }
        ListElement{ xPos: -8.8 ; yPos: 2.96 ; zPos: -3.6 }
        ListElement{ xPos: -6.94 ; yPos: 2.4 ; zPos: -2.92 }
        ListElement{ xPos: -9.02 ; yPos: 4.74 ; zPos: -4.18 }
        ListElement{ xPos: -9.54 ; yPos: 3.1 ; zPos: -3.8 }
        ListElement{ xPos: -6.86 ; yPos: 3.66 ; zPos: -3.58 }
        ListElement{ xPos: -8.16 ; yPos: 1.82 ; zPos: -4.64 }
        ListElement{ xPos: -7.4 ; yPos: 3.18 ; zPos: -4.22 }
        ListElement{ xPos: -7.9 ; yPos: 3.06 ; zPos: -4.3 }
        ListElement{ xPos: -8.98 ; yPos: 2.64 ; zPos: -4.44 }
        ListElement{ xPos: -6.36 ; yPos: 3.96 ; zPos: -4.38 }
        ListElement{ xPos: -7.18 ; yPos: 3.32 ; zPos: -4.04 }
        ListElement{ xPos: -7.9 ; yPos: 3.4 ; zPos: -2.78 }
        ListElement{ xPos: -7.4 ; yPos: 3.12 ; zPos: -3.1 }
        ListElement{ xPos: -7.54 ; yPos: 2.8 ; zPos: -3.68 }
    }

    Scatter3D {
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.MixSeries
        }

        anchors.fill: parent
        cameraPreset: Graphs3D.CameraPreset.Front
        environment: SceneEnvironment {}

        Scatter3DSeries {
            ItemModelScatterDataProxy {
                itemModel: dataModel
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
            }
        }
    }
}
