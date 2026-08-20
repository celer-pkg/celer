// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: "Procedural Geometry Tester"

    SplitView {
        id: mainView
        anchors.fill: parent
        Pane {
            id: editorPane
            objectName: "editorPane"
            SplitView.minimumWidth: 250
            ScrollView {
                id: scrollView
                objectName: "scrollView"
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ColumnLayout {
                    width: scrollView.availableWidth
                    Label {
                        text: "Settings"
                        font.pointSize: 20
                        Layout.alignment: Qt.AlignHCenter
                    }
                    GroupBox {
                        id: typeSelectionGroupBox
                        Layout.fillWidth: true
                        title: "Type"
                        ColumnLayout {
                            width: typeSelectionGroupBox.width

                            RadioButton {
                                id: extrudedTextRadioButton
                                text: "Extruded Text"
                                checked: true
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = extrudedTextGeometry
                                        propertyEditor.setSource("ExtrudedTextSettings.qml", {target: extrudedTextGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: torusRadioButton
                                text: "Torus"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = torusGeometry
                                        propertyEditor.setSource("TorusSettings.qml", {target: torusGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: coneRadioButton
                                text: "Cone"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = coneGeometry
                                        propertyEditor.setSource("ConeSettings.qml", {target: coneGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: cylinderRadioButton
                                text: "Cylinder"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = cylinderGeometry
                                        propertyEditor.setSource("CylinderSettings.qml", {target: cylinderGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: sphereRadioButton
                                text: "Sphere"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = sphereGeometry
                                        propertyEditor.setSource("SphereSettings.qml", {target: sphereGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: planeRadioButton
                                text: "Plane"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = planeGeometry
                                        propertyEditor.setSource("PlaneSettings.qml", {target: planeGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: cuboidRadioButton
                                text: "Cuboid"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = cuboidGeometry
                                        propertyEditor.setSource("CuboidSettings.qml", {target: cuboidGeometry})
                                    }
                                }
                            }
                            RadioButton {
                                id: capsuleRadioButton
                                text: "Capsule"
                                onCheckedChanged: {
                                    if (checked) {
                                        testModel.geometry = capsuleGeometry
                                        propertyEditor.setSource("CapsuleSettings.qml", {target: capsuleGeometry})
                                    }
                                }
                            }
                        }
                    }

                    CheckBox {
                        id: uvCheckerCheckBox
                        text: "Enable UV Checker"
                        checked: false
                        onCheckedChanged: {
                            testMaterial.baseColorMap = checked ? uvCheckerTexture : null
                        }
                    }

                    CheckBox {
                        id: gridCheckBox
                        text: "Enable Grid"
                        checked: true
                    }

                    Loader {
                        id: propertyEditor
                        Layout.fillWidth: true

                        Component.onCompleted: propertyEditor.setSource("ExtrudedTextSettings.qml", {target: extrudedTextGeometry})

                    }
                }
            }
        }

        View3D {
            id: view
            SplitView.fillHeight: true
            SplitView.fillWidth: true

            environment: SceneEnvironment {
                clearColor: "lightblue"
                backgroundMode: SceneEnvironment.Color
                InfiniteGrid {
                    id: infiniteGrid
                    visible: gridCheckBox.checked
                }

            }

            Node {
                id: originNode
                PerspectiveCamera {
                    id: cameraNode
                    z: 200
                }
                DirectionalLight {

                }
            }

            ExtrudedTextGeometry {
                id: extrudedTextGeometry
                text: "test"
            }

            TorusGeometry {
                id: torusGeometry
            }

            ConeGeometry {
                id: coneGeometry
            }

            CylinderGeometry {
                id: cylinderGeometry
            }

            SphereGeometry {
                id: sphereGeometry
            }

            PlaneGeometry {
                id: planeGeometry
            }

            CuboidGeometry {
                id: cuboidGeometry
            }

            CapsuleGeometry {
                id: capsuleGeometry
            }

            Texture {
                id: uvCheckerTexture
                sourceItem: UVChecker {
                    width: 1024
                    height: 1024
                }
            }

            Model {
                id: testModel
                property real spacing
                geometry: extrudedTextGeometry
                materials: [
                    PrincipledMaterial {
                        id: testMaterial
                    }
                ]
            }

            OrbitCameraController {
                id: orbitCameraController
                camera: cameraNode
                origin: originNode

                QuaternionAnimation {
                    id: cameraRotation
                    target: orbitCameraController.origin
                    property: "rotation"
                    duration: 200
                    type: QuaternionAnimation.Slerp
                    running: false
                    loops: 1
                }

                function jumpToAxis(axis) {
                    cameraRotation.from = orbitCameraController.origin.rotation
                    cameraRotation.to = originGizmo.quaternionForAxis(axis)

                    cameraRotation.start()
                }
            }

            OriginGizmo {
                id: originGizmo
                targetNode: cameraNode
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
                width: 100
                height: 100

                onAxisClicked: (axis) => {
                    orbitCameraController.jumpToAxis(axis)
                }
            }

            DebugView {
                source: view
            }
        }
    }
}
