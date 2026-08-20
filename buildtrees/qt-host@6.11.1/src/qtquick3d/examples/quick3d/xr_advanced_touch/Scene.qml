// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Xr
import QtQuick3D.Helpers
import Example

import xr_shared

pragma ComponentBehavior: Bound

Node {
    signal colorChange(col: color, id: int)

    property alias isGrabbing: grabAction.pressed

    //! [color swatch]
    component ColorSwatch: Rectangle {
        implicitWidth: 100
        implicitHeight: 100

        function perceivedBrightness() {
            return 0.2 * color.r + 0.7 * color.g + 0.1 * color.b
        }

        function contrastColor() {
            if (perceivedBrightness() > 0.6)
                return Qt.darker(color, 1.1)

            const h = color.hslHue
            const s = color.hslSaturation
            const l = color.hslLightness + 0.2

            return Qt.hsla(h, s, l, 1.0)
        }

        border.color: contrastColor()
        border.width: csth.pressed ? 8 : 4
        property real penWidth: 5

        TapHandler {
            id: csth
            dragThreshold: 1000
            onTapped: (pt) => {
                          painterItem.setColor(parent.color, pt.id)
                          colorChange(parent.color, pt.id)
                      }
            onLongPressed: {
                painterItem.clear(parent.color)
            }
        }
    }
    //! [color swatch]

    PrincipledMaterial {
        id: material

        baseColorMap: Texture {
            textureData: painterItem.textureData
        }

        roughness: 0.3
        specularAmount: 0.6
    }


    //! [grab action]
    XrInputAction {
        id: grabAction
        controller: XrInputAction.RightHand
        actionId: [ XrInputAction.IndexFingerPinch ]

        onPressedChanged: {
            if (pressed)
                grabController.startGrab()
        }
    }
    //! [grab action]

    //! [grab controller]
    XrController {
        id: grabController
        controller: XrController.RightHand
        property vector3d grabOffset
        property quaternion grabRotation

        function startGrab() {
            const scenePos = selectableModel.scenePosition
            const sceneRot = selectableModel.sceneRotation
            grabOffset = scenePos.minus(scenePosition)
            grabRotation = rotation.inverted().times(sceneRot)
        }

        onRotationChanged: {
            if (isGrabbing) {
                let newPos = scenePosition.plus(grabOffset)
                let newRot = sceneRotation.times(grabRotation)

                selectableModel.setPosition(newPos)
                selectableModel.setRotation(newRot)
            }
        }
    }
    //! [grab controller]

    XrGadget {
        id: selectableModel
        y: -50
        x: 0
        z: -40

        //! [gadget touch]
        onTouched: (uvPosition, touchID, pressed) => {
            if (!isGrabbing)
                painterItem.setUv(uvPosition.x, uvPosition.y, touchID, pressed);
        }
        //! [gadget touch]

        TorusGeometry {
            id: torusG
            radius: 50
            tubeRadius: 20
        }
        CuboidGeometry {
            id: cubeG
        }
        SphereGeometry {
            id: sphereG
            radius: 50
        }
        CylinderGeometry {
            id: cylG
        }
        ConeGeometry {
            id: coneG
        }

        property string meshName: "Torus"

        property var meshMap : {
            "Torus" : torusG,
            "Cube" : cubeG,
            "Sphere" : sphereG,
            "Cylinder" : cylG,
            "Cone" : coneG
        }

        geometry: meshMap[meshName]
        materials: material
        pickable: true

        scale: Qt.vector3d(scaleSlider.value, scaleSlider.value, scaleSlider.value)
    }

    //! [curved screen]
    Model {
        objectName: "curved screen"
        x: 0
        y: -40
        z: -50
        // mesh origin is at bottom of screen; scene origin is at eye height
        geometry: CurvedMesh {
            width: height * 32 / 9
            height: 40
            radius: 80
        }
        visible: true
        pickable: true
        materials: PrincipledMaterial {
            baseColorMap: Texture {
                id: screenTexture
                sourceItem: Rectangle {
                    id: uiItem
                    width: height * 32 / 9
                    height: 500

                    property real dim: Math.min(width, height)

                    color: "#dd000000"

                    ColumnLayout {
                        id: leftItem
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: centralItem.left
                        anchors.margins: 15
                        spacing: 10

                        GridLayout {
                            rows: 3
                            columns: 3
                            columnSpacing: 5
                            rowSpacing: 5

                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                            ColorSwatch {
                                color: "red"
                            }
                            ColorSwatch {
                                color: "orange"
                            }
                            ColorSwatch {
                                color: "yellow"
                            }
                            ColorSwatch {
                                color: "green"
                            }
                            ColorSwatch {
                                color: "blue"
                            }
                            ColorSwatch {
                                color: "purple"
                            }
                            ColorSwatch {
                                color: "white"
                            }
                            ColorSwatch {
                                color: "gray"
                            }
                            ColorSwatch {
                                color: "black"
                            }
                        }
                        RowLayout {
                            Label {
                                text: "Stroke width: " + penWidthSlider.value.toFixed(1)
                            }
                            Slider {
                                id: penWidthSlider
                                Layout.fillWidth: true
                                from: 1
                                to: 30
                                value: 10
                                onValueChanged: painterItem.setPenWidth(value)
                            }
                        }
                    }

                    Rectangle {
                        id: centralItem
                        width: uiItem.dim - 10
                        height: uiItem.dim - 10

                        anchors.centerIn: parent
                        color: "gray"

                        TextureItem {
                            id: painterItem
                            anchors.fill: parent
                            anchors.margins: 2
                            MultiPointTouchArea {
                                anchors.fill: parent
                                onPressed: (list) => {
                                               for (const pt of list) {
                                                   painterItem.setPoint(pt.x, pt.y, pt.pointId, pt.pressed)
                                               }
                                           }
                                onUpdated: (list) => {
                                               for (const pt of list) {
                                                   painterItem.setPoint(pt.x, pt.y, pt.pointId, pt.pressed)
                                               }
                                           }
                                onReleased: (list) => {
                                                for (const pt of list) {
                                                    painterItem.setPoint(pt.x, pt.y, pt.pointId, pt.pressed)
                                                }
                                            }
                            }
                            Component.onCompleted: {
                                // Let initial colors be the same as the hand colors
                                setColor("green", 1)
                                setColor("red", 2)
                            }
                        }
                    }

                    Item {
                        id: rightItem
                        anchors.left: centralItem.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right


                        GroupBox {
                            anchors.centerIn: parent
                            title: "3D Model"
                            ColumnLayout {
                                ColumnLayout {
                                    id: radioButtons
                                    RadioButton {
                                        text: "Torus"
                                        checked: true
                                    }
                                    RadioButton {
                                        text: "Cube"
                                    }
                                    RadioButton {
                                        text: "Sphere"
                                    }
                                    RadioButton {
                                        text: "Cylinder"
                                    }
                                    RadioButton {
                                        text: "Cone"
                                    }
                                }
                                RowLayout {
                                    Label {
                                        text: "Scale: " + scaleSlider.value.toFixed(2)
                                    }
                                    Slider {
                                        id: scaleSlider
                                        Layout.fillWidth: true
                                        from: 0.01
                                        to: 2
                                        value: 0.25
                                    }
                                }
                            }
                            ButtonGroup {
                                buttons: radioButtons.children
                                onCheckedButtonChanged: {
                                    selectableModel.meshName = checkedButton.text
                                }
                            }
                        }
                    }
                }
            }
            emissiveMap: screenTexture
            emissiveFactor: Qt.vector3d(0.8, 0.8, 0.8)
        }
        opacity: 0.99 // enable alpha blending
    }
    //! [curved screen]
}
