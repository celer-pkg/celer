import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("10,000 Cubes (draw call overhead)")

    property int countX: 25
    property int countY: 20
    property int countZ: 20

    property vector3d cubeRotation: Qt.vector3d(0, 0, 0)

    // Uncomment to see performance when animating 10,000 cubes individually
//    NumberAnimation {
//        target: window
//        property: "cubeRotation.x"
//        duration: 1000
//        from: 0
//        to: 360
//        running: true
//        loops: -1
//    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
                GradientStop { position: 0.0; color: "#d2ff52" }
                GradientStop { position: 1.0; color: "#91e842" }
        }
        Text {
            anchors.fill: parent
            text: countX * countY * countZ
            fontSizeMode: Text.Fit
            color: "white"
            minimumPointSize: 128
            font.pointSize: 512
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            renderType: Text.NativeRendering
        }
    }

    Component {
        id: simpleCube
        Model {
            source: "#Cube"
            scale: Qt.vector3d(0.1, 0.1, 0.1)
            eulerRotation: window.cubeRotation
            materials: PrincipledMaterial {
                baseColor: "white"
            }
        }
    }


    View3D {
        id: view
        anchors.fill: parent

        DirectionalLight {
            color: "red"

        }

        DirectionalLight {
            color: "green"
            eulerRotation: Qt.vector3d(0, 180, 0)
        }

        Node {
            id: cameraSpinner
            position: Qt.vector3d(0, 0, 0);


            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 700)
            }

            eulerRotation: Qt.vector3d(0, 90, 0)


            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 10000;
                    from: 0
                    to: 360
                }
            }
        }

        Keys.onPressed: (event)=> {
            switch (event.key) {
                case Qt.Key_C:
                    camera.frustumCullingEnabled = !camera.frustumCullingEnabled
                    break;
                case Qt.Key_Up:
                    camera.position.z -= 100
                    break;
                case Qt.Key_Down:
                    camera.position.z += 100
                    break;
            }
       }
    }

    Timer {
        running: true
        onTriggered: {
            let spacing = 20.0;
            let offsetX = -spacing * countX * 0.5;
            let offsetY = -spacing * countY * 0.5;
            let offsetZ = spacing * countZ * 0.5;

            frustumCullingComponent.createObject(debugView.layout)

            for(var x = 0; x < countX; ++x) {
                for(var y = 0; y < countY; ++y) {
                    for(var z = 0; z < countZ; ++z) {
                        let opacity = (z % 2) ? 1.0 : 0.5;
                        let posX = offsetX + x * spacing;
                        let posY = offsetY + y * spacing;
                        let posZ = offsetZ - z * spacing;

                        simpleCube.createObject(view.scene, {"x": posX, "y": posY, "z": posZ, "opacity": opacity})
                    }
                }
            }
        }
    }

    Component {
        id: frustumCullingComponent
        Text {
           text: "Frustum culling: " + camera.frustumCullingEnabled
           font.pointSize: 9
           color: "white"
       }
    }

    DebugView {
        id: debugView
        source: view
    }
}
