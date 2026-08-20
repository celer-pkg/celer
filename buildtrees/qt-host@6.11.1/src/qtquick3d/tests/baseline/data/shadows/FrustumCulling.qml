import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    width: 1200
    height: 800

    property int numColumns: 2
    property int numRows: 2

    function createView(col, row, cullingEnabled, cameraYawOffset) {
        return Qt.createQmlObject(`
            import QtQuick
            import QtQuick3D
            import QtQuick3D.Helpers

            Item {
                width: parent.width / ${numColumns}
                height: parent.height / ${numRows}
                x: width * ${col}
                y: height * ${row}

                View3D {
                    anchors.fill: parent

                    environment: SceneEnvironment {
                        clearColor: "black"
                    }

                    // Camera is fixed distance, but rotated per view
                    camera: PerspectiveCamera {
                        id: cam
                        position: Qt.vector3d(0, 220, 450)

                        // base angle + per-view offset
                        eulerRotation: Qt.vector3d(-28, 20 + ${cameraYawOffset}, 0)

                        frustumCullingEnabled: ${cullingEnabled}
                    }

                    // Low sun angle, long shadows
                    DirectionalLight {
                        eulerRotation: Qt.vector3d(-15, -70, 0)
                        brightness: 2.5
                        castsShadow: true
                        shadowMapQuality: Light.ShadowMapQualityVeryHigh
                    }

                    // Floor
                    Model {
                        source: "#Cube"
                        position: Qt.vector3d(0, -5, 0)
                        scale: Qt.vector3d(18, 0.05, 18)
                        receivesShadows: true
                        materials: PrincipledMaterial {
                            baseColor: "#bdbdbd"
                        }
                    }

                    // Single cube (fixed world position)
                    Model {
                        source: "#Cube"
                        position: Qt.vector3d(0, 150, 0)
                        castsShadows: true
                        materials: PrincipledMaterial {
                            baseColor: "red"
                        }
                    }
                }

                WasdController {
                    controlledObject: cam
                }

                Rectangle {
                    color: "white"
                    radius: 4
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 5

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        color: "black"
                        font.pixelSize: 14
                        text:
                            "Culling: " + (${cullingEnabled} ? "ON" : "OFF") + "\\n" +
                            "Camera yaw: " + (20 + ${cameraYawOffset})
                    }
                }
            }
        `, window)
    }

    Component.onCompleted: {
        // Row 0
        createView(0, 0, true,  -40)  // cube out of view culled
        createView(1, 0, false, -40)  // cube out of view unculled

        // Row 1
        createView(0, 1, true,  -75)    // cube in view culled
        createView(1, 1, false, -75)    // cube in view unculled
    }
}
