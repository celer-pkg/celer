// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts

import QtQuick3D
import QtQuick3D.Helpers
import QtGraphs
import QtQml

Rectangle {
    color: "black"

    Rectangle {
        id: topLeftOrig
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "transparent"

        Scatter3D {
            id: scatter
            anchors.fill: parent
            anchors.margins: 2
            shadowQuality: Graphs3D.ShadowQuality.None
            selectionMode: Graphs3D.SelectionFlag.Item
            msaaSamples: 4
            aspectRatio: 3.0
            rootNode.scale: Qt.vector3d(10,10,10)
            ambientLightStrength: 0.5

            Scatter3DSeries {
                id: scatterSeries
                ItemModelScatterDataProxy {
                    id: scatterProxy
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
                    xPosRole: "coords"
                    zPosRole: "coords"
                    yPosRole: "data"
                    xPosRolePattern: /(\d),\d/
                    zPosRolePattern: /(\d),(\d)/
                    yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
                    xPosRoleReplace: "\\1"
                    zPosRoleReplace: "\\2"
                    yPosRoleReplace: "\\3"
                }
            }
        }
    }

    //Views
    Rectangle {
        id: topLeft
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width * 0.5
        height: visButton.isEnabled? 0 : parent.height * 0.5
        color: "transparent"

        View3D {
            id: topLeftView
            anchors.fill: parent
            anchors.margins: 2
            camera: camTL
            importScene: scatter.rootNode

            environment: SceneEnvironment {
                clearColor: scatter.theme.backgroundColor
                backgroundMode: SceneEnvironment.Color
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var origin = topLeftView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 0))
                    var far = topLeftView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 1))
                    var direction = Qt.vector3d(far.x - origin.x,
                                                far.y - origin.y,
                                                far.z - origin.z );
                    scatter.doRayPicking(origin, direction)
                }
            }
        }
    }

    Rectangle {
        id: topRight
        anchors.right: parent.right
        anchors.top: parent.top
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "transparent"

        View3D {
            id: topRightView
            anchors.fill: parent
            anchors.margins: 2
            camera: camTR
            importScene: scatter.rootNode

            environment: SceneEnvironment {
                clearColor: scatter.theme.backgroundColor
                backgroundMode: SceneEnvironment.Color
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var origin = topRightView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 0))
                    var far = topRightView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 1))
                    var direction = Qt.vector3d(far.x - origin.x,
                                                far.y - origin.y,
                                                far.z - origin.z );
                    scatter.doRayPicking(origin, direction)
                }
            }
        }
    }

    Rectangle {
        id: bottomLeft
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "transparent"
        border.color: "black"

        View3D {
            id: bottomLeftView
            anchors.fill: parent
            anchors.margins: 2
            camera: camBL
            importScene: scatter.rootNode

            environment: SceneEnvironment {
                clearColor: scatter.theme.backgroundColor
                backgroundMode: SceneEnvironment.Color
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var origin = bottomLeftView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 0))
                    var far = bottomLeftView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 1))
                    var direction = Qt.vector3d(far.x - origin.x,
                                                far.y - origin.y,
                                                far.z - origin.z );
                    scatter.doRayPicking(origin, direction)
                }
            }
        }
    }

    Rectangle {
        id: bottomRight
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width * 0.5
        height: parent.height * 0.5
        color: "transparent"
        border.color: "black"

        View3D {
            id: bottomRightView
            anchors.fill: parent
            anchors.margins: 2
            camera: camBR
            importScene: scatter.rootNode

            environment: SceneEnvironment {
                clearColor: scatter.theme.backgroundColor
                backgroundMode: SceneEnvironment.Color
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var origin = bottomRightView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 0))
                    var far = bottomRightView.mapTo3DScene(Qt.vector3d(mouseX, mouseY, 1))
                    var direction = Qt.vector3d(far.x - origin.x,
                                                far.y - origin.y,
                                                far.z - origin.z );
                    scatter.doRayPicking(origin, direction)
                }
            }
        }
    }

    //Cameras
    PerspectiveCamera {
        id: camTL
        position: Qt.vector3d(0, 10, 50)
        lookAtNode: scatter.rootNode
        eulerRotation.x: -30
        clipNear: 0.001
    }

    OrthographicCamera {
        id: camTR
        y: 20

        verticalMagnification: 5
        horizontalMagnification: 5
        eulerRotation.x: -90
        clipNear: 0.001
    }
    OrthographicCamera {
        id: camBL
        y: 20

        position: Qt.vector3d(0,3,0)
        verticalMagnification: 10
        horizontalMagnification: 10
        clipNear: 0.001
    }
    OrthographicCamera {
        id: camBR
        y: 20

        position: Qt.vector3d(0,3,0)
        verticalMagnification: 10
        horizontalMagnification: 10
        eulerRotation.y: -90
        clipNear: 0.001
    }

    Button {
        id: visButton
        property bool isEnabled: false
        text: isEnabled ? "Hide original graph": "Show original graph"
        onClicked: isEnabled = !isEnabled
    }

}

