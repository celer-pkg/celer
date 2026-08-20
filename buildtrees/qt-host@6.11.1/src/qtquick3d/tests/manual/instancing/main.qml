// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls

import InstancingTest

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("Instanced Rendering")

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "gray"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            PerspectiveCamera {
                z: 1000
                SequentialAnimation  on z {
                    running: false
                    loops: -1
                    PauseAnimation {
                        duration: 4000
                    }
                    NumberAnimation {
                        from: 5000
                        to: 0
                        duration: 15000
                    }
                    PauseAnimation {
                        duration: 300
                    }
                    NumberAnimation {
                        from: 0
                        to: 5000
                        duration: 5000
                    }
                }
            }

            PropertyAnimation on eulerRotation.y {
                from: 0
                to: 360
                running: false
                duration: 17000
                loops: -1
            }
        }

        SpotLight {
            position: Qt.vector3d(0, 800, 0)
            eulerRotation.x: -90
            coneAngle: 60
            color: Qt.rgba(1.0, 1.0, 0.1, 1.0)
            brightness: 50
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            ambientColor: "#777"
            shadowFactor: 90
        }

        CustomInstancing {
            id: customInstancing
            instanceCount: 121
            hasTransparency: true
        }

        Node {
            Model {
                id: transparentModel
                source: "#Cube"
                instancing: customInstancing
                materials: DefaultMaterial {
                    diffuseColor: "lightgray"
                }
            }
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 30000
                loops: -1
            }
        }

        InstanceList {
            id: manualInstancing
            instances: [
                InstanceListEntry {
                    position: Qt.vector3d(0, 0, 0)
                    color: "green"
                },
                InstanceListEntry {
                    position: Qt.vector3d(-300, 0, 200)
                    color: "red"
                    customData: Qt.vector4d(1.0, 0.0, 1.0, 1.0)
                },
                InstanceListEntry {
                    position: Qt.vector3d(200, 200, 100)
                    color: "blue"
                    customData: Qt.vector4d(0.0, 0.3, 1.0, 1.0)
                },
                InstanceListEntry {
                    position: Qt.vector3d(300, 0, 00)
                    eulerRotation: Qt.vector3d(-10, 0, 30)
                    color: "orange"
                    customData: Qt.vector4d(0.0, 0.5, 1.0, 1.0)
                }

            ]
        }

        RandomInstancing {
            id: randomWithData
            instanceCount: 250

            position: InstanceRange {
                from: Qt.vector3d(-500, -400, -500)
                to: Qt.vector3d(500, 400, 500)
            }
            scale: InstanceRange {
                from: Qt.vector3d(0.1, 0.1, 0.1)
                to: Qt.vector3d(0.7, 0.7, 0.7)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            color: InstanceRange {
                from: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                to: Qt.rgba(1, 1, 1, 1.0)
            }
            // instancedMaterial custom data:  METALNESS, ROUGHNESS, FRESNEL_POWER, SPECULAR_AMOUNT
            customData: InstanceRange {
                from: Qt.vector4d(0, 0, 0, 0)
                to: Qt.vector4d(1, 1, 5, 1)
            }

            NumberAnimation on instanceCountOverride {
                from: 1
                to: randomWithData.instanceCount
                duration: 5000
            }
        }

        FileInstancing {
            id: fileInstancing
            source: "test.xml"
        }

        CustomMaterial {
            id: instancedMaterial
            shadingMode: CustomMaterial.Shaded
            fragmentShader: "material.frag"
            vertexShader: "material.vert"
            property color uDiffuse: "lightgray"
        }

        Model {
            id: cubeModel
            source: "#Cube"
            instancing: randomWithData
            //instancing: manualInstancing

            materials: DefaultMaterial {
                diffuseColor: "white"
            }

            PropertyAnimation on eulerRotation.x {
                from: 0
                to: 360
                duration: 5000
                loops: -1
            }
            Model {
                id: sphereModel
                source: "#Sphere"
                instancing: parent.instancing
                instanceRoot: parent

                property real scaleFactor: 0.5
                scale: Qt.vector3d(scaleFactor, scaleFactor, scaleFactor);
                position: Qt.vector3d(-50, -50, -50)
                materials: instancedMaterial

                SequentialAnimation on scaleFactor {
                    PauseAnimation {duration: 3000}
                    NumberAnimation {from: 0.5; to: 1.5; duration: 200}
                    NumberAnimation {from: 1.5; to: 0.5; duration: 200}
                    loops: -1
                }
            } // sphereModel
        } // cubeModel

        Model {
            id: floor
            source: "#Rectangle"
            y: -500
            scale: Qt.vector3d(15, 15, 1)
            eulerRotation.x: -90
            materials: [
                DefaultMaterial {
                    diffuseColor: "pink"
                }
            ]
        }

        Repeater3D {
            model: cubeModel.instancing.instanceCount

            Model {
                id: delegate
                source: "#Cube"
                pickable: true
                property int instanceIndex: index
                position: cubeModel.instancing.instancePosition(index)
                scale: cubeModel.instancing.instanceScale(index)
                rotation: cubeModel.instancing.instanceRotation(index)
                property color instanceColor: cubeModel.instancing.instanceColor(index)

                visible: false
            }
        }

    } // View3D

    MouseArea {
        anchors.fill: viewport

        onClicked: (mouse) => {
            pickPosition.text = "(" + mouse.x + ", " + mouse.y + ")"
            var result = viewport.pick(mouse.x, mouse.y);
            if (result.objectHit) {
                var pickedObject = result.objectHit;

                pickName.text = "Index: " + pickedObject.instanceIndex
                pickColor.color = pickedObject.instanceColor

                uvPosition.text = "("
                        + result.uvPosition.x.toFixed(2) + ", "
                        + result.uvPosition.y.toFixed(2) + ")";
                distance.text = result.distance.toFixed(2);
                scenePosition.text = "("
                        + result.scenePosition.x.toFixed(2) + ", "
                        + result.scenePosition.y.toFixed(2) + ", "
                        + result.scenePosition.z.toFixed(2) + ")";
                localPosition.text = "("
                        + result.position.x.toFixed(2) + ", "
                        + result.position.y.toFixed(2) + ", "
                        + result.position.z.toFixed(2) + ")";
                worldNormal.text = "("
                        + result.sceneNormal.x.toFixed(2) + ", "
                        + result.sceneNormal.y.toFixed(2) + ", "
                        + result.sceneNormal.z.toFixed(2) + ")";
                localNormal.text = "("
                        + result.normal.x.toFixed(2) + ", "
                        + result.normal.y.toFixed(2) + ", "
                        + result.normal.z.toFixed(2) + ")";
                pickInfo.visible = true
            } else {
                pickInfo.visible = false
                pickName.text = "None";
                uvPosition.text = "";
                distance.text = "";
                scenePosition.text = "";
                localPosition.text = "";
                worldNormal.text = "";
                localNormal.text = "";
            }
        }
    }

    Row {
        id: pickInfo
        visible: false
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        spacing: 10
        Column {
            Label {
                color: "white"
                font.pointSize: 14
                text: "Last Pick:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Instance color:"
            }

            Label {
                color: "white"
                font.pointSize: 14
                text: "Screen Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "UV Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Distance:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "World Position:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Local Position:"
            }

            Label {
                color: "white"
                font.pointSize: 14
                text: "World Normal:"
            }
            Label {
                color: "white"
                font.pointSize: 14
                text: "Local Normal:"
            }
        }
        Column {
            Label {
                id: pickName
                color: "white"
                font.pointSize: 14
            }
            Rectangle {
                id: pickColor
                color: "transparent"
                width: pickName.width
                height: pickName.height
            }
            Label {
                id: pickPosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: uvPosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: distance
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: scenePosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: localPosition
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: worldNormal
                color: "white"
                font.pointSize: 14
            }
            Label {
                id: localNormal
                color: "white"
                font.pointSize: 14
            }

        }
    }

    DebugView {
        source: viewport
        anchors.top: parent.top
        anchors.left: parent.left

        TapHandler {
            onTapped: {
                if (cubeModel.instancing == randomWithData) {
                    console.log("clicked: switching instance table to manualInstancing")
                    cubeModel.instancing = manualInstancing
                } else if (cubeModel.instancing == manualInstancing) {
                    console.log("clicked: switching instance table to fileInstancing")
                    cubeModel.instancing = fileInstancing
                } else {
                    console.log("clicked: switching instance table to randomWithData")
                    cubeModel.instancing = randomWithData
                }
            }
        }
        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: {
                customInstancing.depthSortingEnabled = !customInstancing.depthSortingEnabled
                console.log("right clicked: switching depth sorting to", customInstancing.depthSortingEnabled)
            }
        }
    }
}
