// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick3D
import QtQuick3D.Helpers
import QtGraphs
import QtQml

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
Item {
    Bars3D {
        id: bars
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width * 0.5
        aspectRatio: 3.0
        rootNode.scale: Qt.vector3d(10,10,10)

        Bar3DSeries {
            id: barsSeries

            ItemModelBarDataProxy {
                id: barProxy
                itemModel: ListModel {
                    ListElement{ coords: "0,0"; data: "20.0/10.0/4.75"; }
                    ListElement{ coords: "1,0"; data: "21.1/10.3/3.00"; }
                    ListElement{ coords: "2,0"; data: "22.5/10.7/1.24"; }
                    ListElement{ coords: "3,0"; data: "24.0/10.5/2.53"; }
                    ListElement{ coords: "0,1"; data: "20.2/11.2/3.55"; }
                    ListElement{ coords: "1,1"; data: "21.3/11.5/3.03"; }
                    ListElement{ coords: "2,1"; data: "22.6/11.7/3.46"; }
                    ListElement{ coords: "3,1"; data: "23.4/11.5/4.12"; }
                    ListElement{ coords: "0,2"; data: "20.2/12.3/3.37"; }
                    ListElement{ coords: "1,2"; data: "21.1/12.4/2.98"; }
                    ListElement{ coords: "2,2"; data: "22.5/12.1/3.33"; }
                    ListElement{ coords: "3,2"; data: "23.3/12.7/3.23"; }
                    ListElement{ coords: "0,3"; data: "20.7/13.3/5.34"; }
                    ListElement{ coords: "1,3"; data: "21.5/13.2/4.54"; }
                    ListElement{ coords: "2,3"; data: "22.4/13.6/4.65"; }
                    ListElement{ coords: "3,3"; data: "23.2/13.4/6.67"; }
                    ListElement{ coords: "0,4"; data: "20.6/15.0/6.01"; }
                    ListElement{ coords: "1,4"; data: "21.3/14.6/5.83"; }
                    ListElement{ coords: "2,4"; data: "22.5/14.8/7.32"; }
                    ListElement{ coords: "3,4"; data: "23.7/14.3/6.90"; }
                }
                rowRole: "coords"
                columnRole: "coords"
                valueRole: "data"
                rowRolePattern: /(\d),\d/
                columnRolePattern: /(\d),(\d)/
                valueRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\2"
                valueRoleReplace: "\\3"
            }
        }
    }

    View3D {
        id: view3d
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: barVisButton.isEnabled? parent.width * 0.5 : parent.width
        camera: cam
        importScene: bars.rootNode

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
        }

        DirectionalLight {
            eulerRotation.x: -30
        }

        PerspectiveCamera {
            id: cam
            property real xPos: Math.cos(camRot.value * 2 * Math.PI) * 50
            property real zPos: Math.sin(camRot.value * 2 * Math.PI) * 50
            position: Qt.vector3d(
                          xPos,
                          3,
                          zPos
                          )
            lookAtNode: cube
            eulerRotation.x: -30
            clipNear: 0.001
        }

        Model {
            id: cube
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseColor: "red"
            }
            eulerRotation.y: 90
            scale: Qt.vector3d(0.05,0.05,0.05)

            Vector3dAnimation on eulerRotation {
                loops: Animation.Infinite
                duration: 5000
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 0, 360)
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var origin = view3d.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 0))
                var far = view3d.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 1))
                var direction = Qt.vector3d(far.x - origin.x,
                                            far.y - origin.y,
                                            far.z - origin.z );
                bars.doRayPicking(origin, direction)
            }
        }
    }

    Slider {
        anchors.bottom: view3d.bottom
        anchors.left: view3d.left
        anchors.right: view3d.right
        id: camRot
        from: 0
        to: 1
        stepSize: 0.01
        value: 0
    }

    Button {
        id: barVisButton
        property bool isEnabled: false
        text: isEnabled ? "Hide original graph": "Show original graph"
        onClicked: isEnabled = !isEnabled
    }
}
