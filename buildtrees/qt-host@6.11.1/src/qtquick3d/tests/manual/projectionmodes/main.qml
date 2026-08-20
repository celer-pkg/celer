import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    color: "black"

    Node {
        id: sceneRoot
        DirectionalLight {
        }

        Model {
            source: "#Cone"
            materials: DefaultMaterial {
            }
        }

        Model {
            source: "#Sphere"
            z: -100
            x: -100
            materials: DefaultMaterial {
            }
        }

        Model {
            source: "#Cube"
            z: 100
            x: 100
            materials: DefaultMaterial {
            }
        }
    }

    View3D {
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sceneRoot
        PerspectiveCamera {
            id: perspectiveCamera
            z: 600
        }
    }
    View3D {
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sceneRoot

        OrthographicCamera {
            id: orthgraphicCamera
            z: 600
        }
    }
    View3D {
        id: frustumView
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sceneRoot
        FrustumCamera {
            id: frustumCamera
            z: 600
            top: frustumView.height * 0.05
            bottom: frustumView.height * -0.05
            right: frustumView.width * 0.05
            left: frustumView.width * -0.05
        }
    }
    View3D {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sceneRoot
        CustomCamera {
            id: customCamera
            z: 600
            projection: Qt.matrix4x4(1.299, 0, 0, 0,
                                     0, 1.732, 0, 0,
                                     0, 0, -1, -20,
                                     0, 0, -1, 0);
        }
    }

}
