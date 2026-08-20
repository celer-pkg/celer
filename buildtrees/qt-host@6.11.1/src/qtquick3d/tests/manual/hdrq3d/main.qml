// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    View3D {
        id: v3d

        // Render to a texture (Underlay would work too)
        renderMode: View3D.Offscreen
        // Essential for HDR: use a (half-)float backing texture when renderMode is Offscreen
        renderFormat: ShaderEffectSource.RGBA16F

        anchors.fill: parent
        anchors.margins: 16

        environment: SceneEnvironment {
            id: env
        }

        PerspectiveCamera {
            id: camera1
            z: 200
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(2, 2, 2)
            materials: PrincipledMaterial {
                baseColorMap: tex
                lighting: PrincipledMaterial.NoLighting
            }
        }

        Texture {
            id: tex
            source: "file:PeckLake.hdr"
        }

        WasdController {
            controlledObject: camera1
        }
    }

    Rectangle {
        // On Windows all SDR content should be corrected based on the SDR
        // white level queried from the system. E.g. if it's reported as 240
        // nits then colors should be multiplied by 240/80 = 3 (as 1.0 refers
        // to 80 nits). Here there's no query, just a slider.
        layer.enabled: levelCorrection.checked
        layer.format: ShaderEffectSource.RGBA16F
        layer.effect: ShaderEffect {
            fragmentShader: "level.frag.qsb"
            property real multiplier: whiteLevel.value
        }

        color: "gray"
        anchors.right: parent.right
        width: 320
        height: 480
        ColumnLayout {
            GroupBox {
                title: "2D content adjustment"
                ColumnLayout {
                    Label {
                        text: "SDR white level correction\n(relevant if HDR mode is scene-referred, e.g. on Windows)\nMultiplier: " + whiteLevel.value.toFixed(1)
                    }
                    CheckBox {
                        text: "Enable"
                        id: levelCorrection
                    }
                    Slider {
                        id: whiteLevel
                        from: 0
                        to: 10
                        value: 1
                    }
                }
            }
            GroupBox {
                title: "Tonemapping for 3D content (with SDR in mind)"
                ColumnLayout {
                    id: col
                    function updateTonemapping(mode) {
                        env.tonemapMode = mode
                    }
                    RadioButton {
                        text: "Linear (as in linear -> sRGB)"
                        checked: true
                        onCheckedChanged: col.updateTonemapping(SceneEnvironment.TonemapModeLinear)
                    }
                    RadioButton {
                        text: "Aces"
                        onCheckedChanged: col.updateTonemapping(SceneEnvironment.TonemapModeAces)
                    }
                    RadioButton {
                        text: "HejlDawson"
                        onCheckedChanged: col.updateTonemapping(SceneEnvironment.TonemapModeHejlDawson)
                    }
                    RadioButton {
                        text: "Filmic"
                        onCheckedChanged: col.updateTonemapping(SceneEnvironment.TonemapModeFilmic)
                    }
                    RadioButton {
                        text: "None"
                        onCheckedChanged: col.updateTonemapping(SceneEnvironment.TonemapModeNone)
                    }
                }
            }
            Label {
                function renderFormatStr() {
                    switch (v3d.renderFormat) {
                    case ShaderEffectSource.RGBA8:
                        return "RGBA8";
                    case ShaderEffectSource.RGBA16F:
                        return "RGBA16F";
                    case ShaderEffectSource.RGBA32F:
                        return "RGBA32F";
                    }
                }
                text: "View3D renderFormat is " + renderFormatStr()
            }
        }
    }
}
