// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "Custom Post-Processing Effect Example"
    color: "#848895"

    View3D {
        id: view3d

        //! [eff1]
        Effect {
            id: eff1
            property TextureInput tex: TextureInput {
                id: qtLogo
                texture: Texture { source: "qt_logo_rect.png" }
            }
            passes: Pass {
                shaders: Shader {
                    id: fs1
                    stage: Shader.Fragment
                    shader: "effect.frag"
                }
            }
        }
        //! [eff1]

        //! [eff2]
        Effect {
            id: eff2
            property real uRed: 0.0
            SequentialAnimation {
                running: radioEff2.checked || radioEff3.checked
                loops: Animation.Infinite
                NumberAnimation { target: eff2; property: "uRed"; from: 0; to: 1; duration: 2000 }
                NumberAnimation { target: eff2; property: "uRed"; from: 1; to: 0; duration: 2000 }
            }
            property real uGreen: 1.0
            Shader {
                id: vs2
                stage: Shader.Vertex
                shader: "effect2.vert"
            }
            Shader {
                id: fs2
                stage: Shader.Fragment
                shader: "effect2.frag"
            }
            passes: Pass {
                shaders: [ vs2, fs2 ]
            }
        }
        //! [eff2]

        Effect {
            id: eff3

            property TextureInput tex: qtLogo
            property real uRed: 1.0
            property real uGreen: 0.0
            SequentialAnimation {
                running: radioEff4.checked
                loops: Animation.Infinite
                NumberAnimation { target: eff3; property: "uGreen"; from: 0; to: 1; duration: 2000 }
                NumberAnimation { target: eff3; property: "uGreen"; from: 1; to: 0; duration: 2000 }
            }

            Buffer {
                id: intermediateTexture
                name: "intermediateTexture"
                format: Buffer.RGBA8
                textureFilterOperation: Buffer.Linear
                textureCoordOperation: Buffer.ClampToEdge
                sizeMultiplier: 2 // just for fun upscale and then downscale
            }

            passes: [
                Pass {
                    shaders: [ fs1 ]
                    output: intermediateTexture
                },
                Pass {
                    shaders: [ vs2, fs2 ]
                    commands: [
                        BufferInput {
                            buffer: intermediateTexture
                        }
                    ]
                }
            ]
        }

        anchors.fill: parent
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            id: env
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
            effects: [ eff1 ]
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -20
        }

        DirectionalLight {
            eulerRotation.x: -20
            eulerRotation.y: 20
            ambientColor: Qt.rgba(0.8, 0.8, 0.8, 1.0);
        }

        Texture {
            id: checkers
            source: "checkers2.png"
            scaleU: 20
            scaleV: 20
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }

        Model {
            source: "#Rectangle"
            scale.x: 10
            scale.y: 10
            eulerRotation.x: -90
            materials: [ PrincipledMaterial { baseColorMap: checkers } ]
        }

        Model {
            source: "#Cone"
            position: Qt.vector3d(100, 0, -200)
            scale.y: 3
            materials: [ PrincipledMaterial { baseColor: "green" } ]
        }

        Model {
            id: sphere
            source: "#Sphere"
            position: Qt.vector3d(-100, 200, -200)
            materials: [ PrincipledMaterial { baseColor: "#808000" } ]
        }

        Model {
            source: "#Cube"
            position.y: 50
            eulerRotation.y: 20
            materials: [ PrincipledMaterial { baseColor: "gray" } ]
        }
    }

    WasdController {
        controlledObject: camera
    }

    ColumnLayout {
        Label {
            text: "Use WASD and mouse to navigate"
            font.bold: true
        }
        ButtonGroup {
            buttons: [ radioEff1, radioEff2, radioEff3, radioEff4, radioEff5 ]
        }
        // qmllint disable read-only-property
        // QTBUG-114144
        RadioButton {
            id: radioEff1
            text: "Custom effect with fragment shader only"
            checked: true
            focusPolicy: Qt.NoFocus
            onCheckedChanged: {
                if (checked)
                    env.effects = [ eff1 ];
            }
        }
        RadioButton {
            id: radioEff2
            text: "Custom effect with vertex and fragment shaders"
            checked: false
            focusPolicy: Qt.NoFocus
            onCheckedChanged: {
                if (checked)
                    env.effects = [ eff2 ];
            }
        }
        RadioButton {
            id: radioEff3
            text: "Both effects chained"
            checked: false
            focusPolicy: Qt.NoFocus
            onCheckedChanged: {
                if (checked)
                    env.effects = [ eff1, eff2 ];
            }
        }
        RadioButton {
            id: radioEff4
            text: "As one single, multi-pass effect"
            checked: false
            focusPolicy: Qt.NoFocus
            onCheckedChanged: {
                if (checked)
                    env.effects = [ eff3 ];
            }
        }
        RadioButton {
            id: radioEff5
            text: "No effects"
            checked: false
            focusPolicy: Qt.NoFocus
            onCheckedChanged: {
                if (checked)
                    env.effects = [];
            }
        }
        // qmllint disable read-only-property
    }
}
