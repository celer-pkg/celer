import QtQuick
import QtQuick.Shapes
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    id: window
    width: 800
    height: 480

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: originNode
            y: 150
            PerspectiveCamera {
                id: cameraNode
                z: 100
            }
        }
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Node {
            y: 150
            x: -50
            Rectangle {
                x: 5
                y: -25
                width: 50
                height: 50
                color: "lightgreen"
                Shape {
                    anchors.fill: parent
                    preferredRendererType: Shape.CurveRenderer
                    ShapePath {
                        strokeColor: "black"
                        PathRectangle {
                            x: 0
                            y: 0
                            width: 50
                            height: 50
                            topLeftRadius: 25
                        }
                    }
                }
            }
            Rectangle {
                x: 60
                y: -25
                width: 50
                height: 50
                color: "#ffffbb"
                Shape {
                    anchors.fill: parent
                    preferredRendererType: Shape.CurveRenderer
                    ShapePath {
                        fillColor: "black"
                        strokeColor: "transparent"
                        PathRectangle {
                            x: 0
                            y: 0
                            width: 50
                            height: 50
                            topLeftRadius: 25
                        }
                    }
                }
            }
        }
    } // View3D
} // Window
