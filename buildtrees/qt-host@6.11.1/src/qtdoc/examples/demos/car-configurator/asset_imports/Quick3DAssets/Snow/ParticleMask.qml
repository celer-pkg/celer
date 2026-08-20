// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Particles3D
import QtQuick3D.Helpers
import Quick3DAssets.Snow

View3D {
    id: particleMaskView
    width: 2048
    height: 2048
    property alias cameraPosition: particleMaskCamera.position
    property alias clipNear: particleMaskCamera.clipNear
    property alias clipFar: particleMaskCamera.clipFar
    property alias viewProjection: particleMaskCamera.viewProjection
    property alias brushPositionInUV: particleMaskShader.brushPositionInUV
    property alias brushSize: particleMaskShader.brushSize
    property alias maskSource: particleMaskSource
    property alias particleSize: snowParticleSystem.spriteScale
    property alias nodePosition: node.position
    property alias snowParticleSystemVisible: snowParticleSystem.visible

    camera: particleMaskCamera

    Node {
        id: node
        OrthographicCamera {
            id: particleMaskCamera
            y: 350.
            clipNear: 240.
            clipFar: 260.
            eulerRotation.x: -90
            horizontalMagnification: 1
            verticalMagnification: 1

            property real overrideNear: 1
            property real overrideFar: 2000

            property matrix4x4 viewProjection: Qt.matrix4x4()
            onScenePositionChanged: {
                viewProjection = viewProjectionMatrix();
            }
            onSceneRotationChanged: {
                viewProjection = viewProjectionMatrix();
            }
            onHorizontalMagnificationChanged: {
                viewProjection = viewProjectionMatrix();
            }
            onVerticalMagnificationChanged: {
                viewProjection = viewProjectionMatrix();
            }

            function projectionMatrix() {
                let width = particleMaskView.width * particleMaskCamera.horizontalMagnification;
                let invHeight = 1. - particleMaskView.height * particleMaskCamera.verticalMagnification;
                let near = particleMaskCamera.overrideNear;
                let far = particleMaskCamera.overrideFar;
                let top = -invHeight * 0.5;
                let bottom = invHeight * 0.5;
                let left = -width * 0.5;
                let right = width * 0.5;

                return Qt.matrix4x4(2 / width, 0, 0, -(left + right) / width, 0, 2 / invHeight, 0, -(top + bottom) / invHeight, 0, 0, -2 / (far - near), -(near + far) / (far - near), 0, 0, 0, 1);
            }
            function viewMatrix() {
                return particleMaskCamera.sceneTransform.inverted();
            }
            function viewProjectionMatrix() {
                return projectionMatrix().times(viewMatrix());
            }
        }
        SnowParticlesSystem {
            id: snowParticleSystem
            spriteScale: 6
        }
    }

    environment: SceneEnvironment {
        clearColor: "black"
        tonemapMode: SceneEnvironment.TonemapModeNone
        effects: [
            Effect {
                id: particleMaskShader
                property vector2d brushPositionInUV: Qt.vector2d(-10., -10.)
                property real brushSize: 0.03
                property TextureInput lastColorTexture: TextureInput {
                    texture: Texture {
                        sourceItem: ShaderEffectSource {
                            id: particleMaskSource
                            sourceItem: particleMaskView
                            hideSource: true
                            textureMirroring: GraphicsInfo.api == GraphicsInfo.OpenGL ? ShaderEffectSource.NoMirroring : ShaderEffectSource.MirrorVertically
                        }
                    }
                }
                passes: Pass {
                    shaders: Shader {
                        stage: Shader.Fragment
                        shader: "qrc:/qt/qml/Quick3DAssets/Snow/snow_mask_effect.frag"
                    }
                }
            }
        ]
    }
}
