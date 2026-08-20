import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers

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
            position: Qt.vector3d(0, 50, 0)
            collisionShapes: [boxshape, sphereshape]
        }

        StaticRigidBody {
            id: body1
            position: Qt.vector3d(0, 150, 0)
            collisionShapes: [sphereshape, boxshape]
        }

        StaticRigidBody {
            id: body2
            position: Qt.vector3d(0, 250, 0)
            collisionShapes: [capsuleshape, sphereshape]
        }

        StaticRigidBody {
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
        }
    }

    WasdController {
        controlledObject: camera
    }

    Timer {
        interval: 1
        running: true
        repeat: false
        onTriggered: {
            body0.collisionShapes.pop()
            body1.collisionShapes.pop()
            body2.collisionShapes.pop()
        }
    }
}
