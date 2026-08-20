import QtQuick
import QtQuick3D
import QtQuick3D.Physics

// Two bodies where one is scaled 1.5x*2x and the other 3x
// and they should have the same size.

Rectangle {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
        forceDebugDraw: true
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
            eulerRotation: Qt.vector3d(10, 20, 30)
            scale: Qt.vector3d(1, 1, 1).times(3)
            collisionShapes: BoxShape {
                position: Qt.vector3d(0, -100, 0)
            }
        }

        Node {
            scale: Qt.vector3d(1, 1, 1).times(2)
            StaticRigidBody {
                eulerRotation: Qt.vector3d(10, 20, 30)
                scale: Qt.vector3d(1, 1, 1).times(1.5)
                collisionShapes: BoxShape {
                    position: Qt.vector3d(0, 100, 0)
                }
            }
        }
    }
}
