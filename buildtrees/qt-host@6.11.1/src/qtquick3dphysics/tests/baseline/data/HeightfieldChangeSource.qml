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
            hfShape0.image = null
            hfShape1.image = null
        }
    }

    View3D {
        id: viewport
        anchors.fill: parent

        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 200)
            eulerRotation: Qt.vector3d(-25, 0, 0)
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
            id: tablecloth0
            position: Qt.vector3d(-80, 0, 0)
            collisionShapes: HeightFieldShape {
                id: hfShape0
                extents: Qt.vector3d(150, 20, 150)
                source: "cloth-heightmap.png"
                image: Image {
                    source: "cloth-heightmap-small.png"
                }
            }
        }

        StaticRigidBody {
            id: tablecloth1
            position: Qt.vector3d(80, 0, 0)
            collisionShapes: HeightFieldShape {
                id: hfShape1
                extents: Qt.vector3d(150, 20, 150)
                source: "cloth-heightmap-small.png"
                image: Image {
                    source: "cloth-heightmap.png"
                }
            }
        }
    }
}
