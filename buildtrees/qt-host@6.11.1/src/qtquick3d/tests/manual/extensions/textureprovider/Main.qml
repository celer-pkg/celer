// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import QtQuick3D
import QtQuick3DTest.Extensions.TextureProvider

Window {
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("Texture provider")

    View3D {
        id: view
        anchors.left: settings.right
        anchors.right: parent.right
        height: parent.height

        property int api: GraphicsInfo.api
        property bool yIsPointingUpInTextures: view.api === GraphicsInfo.OpenGL
        property real apiFactor: yIsPointingUpInTextures ? 1.0 : 0.0
        property real apiFactorFlip: 1.0 - apiFactor

        extensions: [ RenderExtension {
                id: renderer
            } ]

        Texture {
            id: rendererTexture
            textureProvider: renderer
            flipV: !view.yIsPointingUpInTextures // ignored by custom mat. and postproc. effects
        }

        camera: PerspectiveCamera {
            z: 600
        }

        DirectionalLight {

        }

        Model {
            visible: principledMaterialOption.checked
            source: "#Cube"
            materials: [ PrincipledMaterial {
                baseColorMap: rendererTexture
            } ]

            SequentialAnimation on eulerRotation.y {
                running: animateCb.checked
                NumberAnimation { from: 0; to: 360; duration: 4000 }
                loops: -1
            }
        }

        environment: SceneEnvironment {
            id: env
            effects: []
        }

        Effect {
            id: simpleEffect
            property TextureInput extensionTexture: TextureInput {
                enabled: textureInputEnableCb.checked
                texture: rendererTexture
            }
            property real apiFactor: view.apiFactor
            property real apiFactorFlip: view.apiFactorFlip
            passes: Pass {
               shaders: Shader {
                   stage: Shader.Fragment
                   shader: "effects/effect.frag"
               }
            }
        }

        Model {
            visible: customMaterialShadedOption.checked
            source: "#Cube"
            scale: Qt.vector3d(2, 2, 2)
            materials: CustomMaterial {
                fragmentShader: "materials/shaded_custommaterial.frag"
                property TextureInput extensionTexture: TextureInput {
                    enabled: textureInputEnableCb.checked
                    texture: rendererTexture
                }
                property real apiFactor: view.apiFactor
                property real apiFactorFlip: view.apiFactorFlip
            }
            SequentialAnimation on eulerRotation.y {
                running: animateCb.checked
                NumberAnimation { from: 0; to: 360; duration: 4000 }
                loops: -1
            }
        }

        Model {
            visible: customMaterialUnshadedOption.checked
            source: "#Cube"
            scale: Qt.vector3d(3, 3, 3)
            materials: CustomMaterial {
                shadingMode:  CustomMaterial.Unshaded
                vertexShader: "materials/unshaded_custommaterial.vert"
                fragmentShader: "materials/unshaded_custommaterial.frag"
                property TextureInput extensionTexture: TextureInput {
                    enabled: textureInputEnableCb.checked
                    texture: rendererTexture
                }
                property real apiFactor: view.apiFactor
                property real apiFactorFlip: view.apiFactorFlip
            }
            SequentialAnimation on eulerRotation.y {
                running: animateCb.checked
                NumberAnimation { from: 0; to: 360; duration: 4000 }
                loops: -1
            }
        }
    }

    Rectangle {
        id: settings
        anchors.left: parent.left
        anchors.top: parent.top
        implicitWidth: 350
        implicitHeight: 210
        anchors.margins: 10
        opacity: 0.6
        color: "gray"
        ColumnLayout {
            RadioButton {
                id: principledMaterialOption
                text: "PrincipledMaterial (baseColorMap)"
                checked: true
            }

            RadioButton {
                id: customMaterialShadedOption
                text: "Shaded custom material"
            }

            RadioButton {
                id: customMaterialUnshadedOption
                text: "Unshaded custom material (linear, no tonemap)"
            }

            RadioButton {
                id: effectOption
                text: "Post-processing effect"
                onCheckedChanged: {
                    if (checked)
                        env.effects.push(simpleEffect);
                    else
                        env.effects = [];
                }
            }
            CheckBox {
                id: textureInputEnableCb
                text: "TextureInput enabled"
                checked: true
            }
            CheckBox {
                id: animateCb
                text: "Animate"
                checked: false
            }

            Button {
                text: "Release resources"
                onClicked: window.releaseResources()
            }
        }
    }
}
