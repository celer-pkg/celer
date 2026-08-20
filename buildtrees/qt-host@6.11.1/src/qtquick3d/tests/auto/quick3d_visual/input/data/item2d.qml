import QtQuick
import QtQuick3D

View3D {
    width: 1024; height: 480

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    environment: SceneEnvironment {
        clearColor: "#111"
        backgroundMode: SceneEnvironment.Color
    }

    PerspectiveCamera {
        z: 600
    }

    DirectionalLight {

    }

    DirectionalLight {
        eulerRotation.y: -90
    }

    Node {
        objectName: "left object"
        x: -256
        y: 128
        z: 380
        eulerRotation.y: 15
        BusyBox {
            objectName: "left busybox"
        }
    }

    Node {
        objectName: "right object"
        x: 32
        y: 128
        z: 300
        eulerRotation.y: -5
        BusyBox {
            objectName: "right busybox"
        }
    }
}
