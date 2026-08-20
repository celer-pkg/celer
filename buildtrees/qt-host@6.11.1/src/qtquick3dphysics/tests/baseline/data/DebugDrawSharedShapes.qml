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
        BoxShape {
            id: boxshape
        }
        SphereShape {
            id: sphereshape
        }
        CapsuleShape {
            id: capsuleshape
        }
        StaticRigidBody {
            id: body0
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: [boxshape, sphereshape, capsuleshape]
        }
        StaticRigidBody {
            id: body1
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: [boxshape, sphereshape, capsuleshape]
        }
    }
}

