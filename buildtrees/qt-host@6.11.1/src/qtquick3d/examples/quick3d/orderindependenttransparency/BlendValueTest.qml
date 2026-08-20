// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Item {
    id: mainWindow
    anchors.fill: parent

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
            oitMethod: method.index
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
        }

        Node {
            id: sceneRoot
            eulerRotation.y: cameraRotation.sliderValue
            Texture {
                id: modelTexture
                source: "images/qt_logo.png"
                magFilter: Texture.Nearest
            }
            Texture {
                id: noTexture
            }
            Model {
                id: blackrect
                source: "#Rectangle"
                scale: Qt.vector3d(10.0, 10.0, 1)
                position: Qt.vector3d(0, 0, -250)
                materials: DefaultMaterial {
                    diffuseColor: "white"
                    diffuseMap: Texture {
                        source: "images/tilepattern.png"
                        magFilter: Texture.Nearest
                    }
                    lighting: DefaultMaterial.NoLighting
                    opacity: 1
                }
            }
            Model {
                id: s1
                source: "#Sphere"
                scale: Qt.vector3d(3.0, 3.0, 0.01)
                position: Qt.vector3d(-50, 50, -200)
                materials: DefaultMaterial {
                    diffuseColor: "red"
                    lighting: DefaultMaterial.NoLighting
                    opacity: 0.5
                    diffuseMap: textureCheckBox.checked ? modelTexture : noTexture
                }
            }
            Model {
                id: s2
                source: "#Sphere"
                scale: Qt.vector3d(3.0, 3.0, 0.01)
                position: Qt.vector3d(50, 50, -150)
                materials: DefaultMaterial {
                    diffuseColor: "green"
                    lighting: DefaultMaterial.NoLighting
                    opacity: 0.5
                    diffuseMap: textureCheckBox.checked ? modelTexture : noTexture
                }
            }
            Model {
                id: s3
                source: "#Sphere"
                scale: Qt.vector3d(3.0, 3.0, 0.01)
                position: Qt.vector3d(0, -50, -100)
                materials: DefaultMaterial {
                    diffuseColor: "blue"
                    lighting: DefaultMaterial.NoLighting
                    opacity: 0.5
                    diffuseMap: textureCheckBox.checked ? modelTexture : noTexture
                }
            }
        }
    }

    SettingsView {
        id: settingsView
        CustomLabel {
            id: methodLabel
            text: method.selection
        }
        CustomSelectionBox {
            id: method
            text: "OIT Method"
            values: ["None", "WeightedBlended", "LinkedList"]
            parentWidth: settingsView.width
        }
        CustomLabel {
            text: "Camera rotation"
        }
        CustomSlider {
            id: cameraRotation
            fromValue: -85
            toValue: 85
            sliderValue: 0
        }
        CustomLabel {
            text: "Enable texture"
        }
        CustomCheckBox {
            id: textureCheckBox
            checked: false
        }
    }
    CustomInfoView {
        width: settingsView.width
        anchors.top: settingsView.bottom
        anchors.topMargin: 30
        x: parent.width - settingsView.width
        anchors.bottom: parent.bottom
        visible: AppSettings.showSettingsView
        selection: method.index

        titles: [
            "OIT Disabled",
            "Weighted blended",
            "Linked List"
        ]
        contents: [
            "When order independent transparency\nis disabled, the drawing order is deter-\nmined by the center or the object.\nWhen rotating the camera, drawing\norder will change.",
            "When order independent transparency\nis enabled, the drawing order doesn't\naffect the render result anymore.\nInstead it is only affected by the\nz-coordinate of the pixel. Weighted\nblended method doesn't use the porter-\nduff source over blending so the\nblending results are different.",
            "When using Linked list method, the\nrendered fragments are placed in\na linked list. In the second pass\nthey are sorted and rendered in the correct order."
        ]
    }
}
