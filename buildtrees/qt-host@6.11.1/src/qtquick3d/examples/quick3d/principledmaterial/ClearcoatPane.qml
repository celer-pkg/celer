// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D

// qmllint disable missing-property
// Disabling missing-property because the targetMaterial property
// will either be a PrincipaledMaterial or SpecularGlossyMaterial
// but the shared properties are not part of the common base class
ScrollView {
    id: rootView
    required property Material targetMaterial
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    width: availableWidth

    ColumnLayout {
        width: rootView.availableWidth

        MarkdownLabel {
            text: `# Clearcoat
Clearcoat when enabled gives the appearance of another protective layer on top
of the existing material. A real life analog would be a clear lacquer.

## Clearcoat Amount
To enable the clearcoat layer, the value of Clearcoat amount must be change to
something greater than 0.0. There is a cost for adding a clearcoat layer
(basically doing all specular lighting for a second time) so is disabled by
default.
`
        }

        RowLayout {
            Label {
                text: "Clearcoat Amount (" + rootView.targetMaterial.clearcoatAmount.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.clearcoatAmount
                onValueChanged: rootView.targetMaterial.clearcoatAmount = value
            }
        }

        MarkdownLabel {
            text: `### Clearcoat Fresnel Power
This property decreases head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
        }
        RowLayout {
            Label {
                text: "Clearcoat Fresnel Power  (" + (rootView.targetMaterial.clearcoatFresnelPower !== undefined ? rootView.targetMaterial.clearcoatFresnelPower.toFixed(2) : 5.0) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0.0
                to: 10.0
                value: (rootView.targetMaterial.clearcoatFresnelPower !== undefined ? rootView.targetMaterial.clearcoatFresnelPower : 5.0)
                onValueChanged: rootView.targetMaterial.clearcoatFresnelPower = value
            }
        }

        MarkdownLabel {
            text: `## Enable Clearcoat Fresnel ScaleBias
The material will take Clearcoat Fresnel Scale and Clearcoat Fresnel Bias into account.`
        }
        RowLayout {
            Label {
                text: "Enable Clearcoat Fresnel ScaleBias"
            }
            Switch {
                checked: rootView.targetMaterial.clearcoatFresnelScaleBiasEnabled
                onCheckedChanged: {
                    clearcoatFresnelScaleLabel.enabled = checked
                    clearcoatFresnelScale.enabled = checked
                    clearcoatFresnelBiasLabel.enabled = checked
                    clearcoatFresnelBias.enabled = checked
                    rootView.targetMaterial.clearcoatFresnelScaleBiasEnabled = checked
                }
            }
        }

        MarkdownLabel {
            id: clearcoatFresnelScaleLabel
            enabled: false
            text: `### Clearcoat Fresnel Scale
This property scale head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
        }
        RowLayout {
            id: clearcoatFresnelScale
            enabled: false
            Label {
                text: "Clearcoat Fresnel scale  (" + (rootView.targetMaterial.clearcoatFresnelScale !== undefined ? rootView.targetMaterial.clearcoatFresnelScale.toFixed(2) : 1.0) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0.0
                to: 10.0
                value: (rootView.targetMaterial.clearcoatFresnelScale !== undefined ? rootView.targetMaterial.clearcoatFresnelScale : 1.0)
                onValueChanged: rootView.targetMaterial.clearcoatFresnelScale = value
            }
        }

        MarkdownLabel {
            id: clearcoatFresnelBiasLabel
            enabled: false
            text: `### Clearcoat Fresnel Bias
This property push forward head-on reflections (looking directly at the
surface) while maintaining reflections seen at grazing angles.`
        }
        RowLayout {
            id: clearcoatFresnelBias
            enabled: false
            Label {
                text: "Clearcoat Fresnel Bias  (" + (rootView.targetMaterial.clearcoatFresnelBias !== undefined ? rootView.targetMaterial.clearcoatFresnelBias.toFixed(2) : 0.0) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: -1.0
                to: 1.0
                value: (rootView.targetMaterial.clearcoatFresnelBias !== undefined ? rootView.targetMaterial.clearcoatFresnelBias : 0.0)
                onValueChanged: rootView.targetMaterial.clearcoatFresnelBias = value
            }
        }

        MarkdownLabel {
            text: `### Clearcoat Map
The Clearcoat Map is a single channel texture in which when multiplied against
the Clearcoat Amount property determines what the clearcoat amount is for the
material.`
        }
        ComboBox {
            id: clearcoatChannelComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.clearcoatChannel = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.clearcoatChannel)
            model: [
                { value: PrincipledMaterial.R, text: "Red Channel"},
                { value: PrincipledMaterial.G, text: "Green Channel"},
                { value: PrincipledMaterial.B, text: "Blue Channel"},
                { value: PrincipledMaterial.A, text: "Alpha Channel"}
            ]
        }

        TextureSourceControl {
            defaultTexture: "maps/noise.png"
            defaultClearColor: "black"
            onTargetTextureChanged: {
                rootView.targetMaterial.clearcoatMap = targetTexture
            }
        }

        MarkdownLabel {
            text: `## Clearcoat Roughness
Just like the base material layer, the Clearcoat layer also has has a roughness
property. The Clearcoat Roughness property will make the specular reflection of
the clearcoat layer more diffuse.`
        }

        RowLayout {
            Label {
                text: "Clearcoat Roughness (" + rootView.targetMaterial.clearcoatRoughnessAmount.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.clearcoatRoughnessAmount
                onValueChanged: rootView.targetMaterial.clearcoatRoughnessAmount = value
            }
        }

        MarkdownLabel {
            text: `### Clearcoat Roughness Map
The Clearcoat Roughness Map is a single channel texture in which when multiplied
against the Clearcoat Roughness Amount property determines the roughness of the clearcoat
layer for the material.`
        }

        ComboBox {
            id: clearcoatRoughnessChannelComboBox
            textRole: "text"
            valueRole: "value"
            implicitContentWidthPolicy: ComboBox.WidestText
            onActivated: rootView.targetMaterial.clearcoatRoughnessChannel = currentValue
            Component.onCompleted: currentIndex = indexOfValue(rootView.targetMaterial.clearcoatRoughnessChannel)
            model: [
                { value: PrincipledMaterial.R, text: "Red Channel"},
                { value: PrincipledMaterial.G, text: "Green Channel"},
                { value: PrincipledMaterial.B, text: "Blue Channel"},
                { value: PrincipledMaterial.A, text: "Alpha Channel"}
            ]
        }

        TextureSourceControl {
            defaultTexture: "maps/noise.png"
            defaultClearColor: "black"
            onTargetTextureChanged: {
                rootView.targetMaterial.clearcoatRoughnessMap = targetTexture
            }
        }


        MarkdownLabel {
            text: `## Clearcoat Normal Map
Finally, the clearcoat layer also supports applying a normal map to further
define the surface details. This normal map works the same way as the Normal
Map property of PrincipledMaterial, just only applies to the clearcoat layer.
It is possible that the material would have two normal maps applied in this
case.
`
        }

        TextureSourceControl {
            defaultTexture: "maps/metallic/normal.jpg"
            defaultClearColor: Qt.rgba(0.5, 0.5, 1.0, 1.0)
            stampMode: true
            stampSource: "maps/normal_stamp.png"
            onTargetTextureChanged: {
                rootView.targetMaterial.clearcoatNormalMap = targetTexture
            }
        }
        MarkdownLabel {
            text: `### Clearcoat Normal Strength
By adjusting the normal strength you will see that the amount of influence the
normal map has on the material changes.`
        }
        RowLayout {
            Label {
                text: "Clearcoat Normal Strength (" + rootView.targetMaterial.clearcoatNormalStrength.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                from: 0
                to: 1
                value: rootView.targetMaterial.clearcoatNormalStrength
                onValueChanged: rootView.targetMaterial.clearcoatNormalStrength = value
            }
        }
    }
}
// qmllint enable missing-property
