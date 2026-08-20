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
        running: true
    }
    View3D {
        id: viewport
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 50, 500)
            eulerRotation: Qt.vector3d(0, 0, 0)
            clipFar: 5000
            clipNear: 1
        }
        DirectionalLight {}

        StaticRigidBody {
            position: Qt.vector3d(-150, 0, 0)
            collisionShapes: BoxShape {}
        }
        StaticRigidBody {
            position: Qt.vector3d(-50, 0, 0)
            collisionShapes: SphereShape {}
        }
        StaticRigidBody {
            position: Qt.vector3d(50, 0, 0)
            collisionShapes: CapsuleShape {}
        }
        CharacterController {
            position: Qt.vector3d(150, 0, 0)
            collisionShapes: CapsuleShape {}
        }
    }
}

