import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Rectangle {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
        forceDebugDraw: true
    }

    Timer {
        interval: 1; running: true; repeat: false
        onTriggered: {
            tablecloth.scale = Qt.vector3d(10, 10, 10)
        }
    }

    View3D {
        id: viewport
        anchors.fill: parent

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 1200)
            eulerRotation: Qt.vector3d(0, 0, 0)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 100
        }

        StaticRigidBody {
            id: tablecloth
            collisionShapes: HeightFieldShape {
                id: hfShape
                extents: Qt.vector3d(150, 20, 150)
                source: "cloth-heightmap.png"
            }
        }
    }
}
